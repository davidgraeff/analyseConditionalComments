#include "analysermanager.h"
#include "cfileanalyser.h"
#include <QApplication>
#include <QMutexLocker>
#include <QSettings>
#include <QtCore>
#include <QFontMetrics>

AnalyserManager::AnalyserManager(QObject *parent)
{
  m_filters << QLatin1String("*.cpp") << QLatin1String("*.h") << QLatin1String("*.c") << QLatin1String("*.hpp");
  connect(&m_fileresultFuture, SIGNAL(canceled()), SLOT(analyseCanceled()));
  connect(&m_fileresultFuture, SIGNAL(finished()), SLOT(allFilesAnalysed()));
  connect(&m_fileresultFuture, SIGNAL(progressValueChanged(int)), SIGNAL(progressValueChanged(int)));
  connect(&m_resultSVGFutureWatcher, SIGNAL(canceled()), SIGNAL(svgCanceled()));
  connect(&m_resultSVGFutureWatcher, SIGNAL(finished()), SLOT(generateSVGFinished()));

  m_fileBackgroundColor = QColor(255,255,255,255); // transparent
  m_fileMarkDefaultColor = QColor(0,0,255,40);
  clear();
}

AnalyserManager::~AnalyserManager() {
  m_results.cancel();
  m_results.waitForFinished();
  clear();
}

QColor AnalyserManager::fileMarkDefaultColor() const {
    return m_fileMarkDefaultColor;
}

void AnalyserManager::cancelAnalysingFiles() {
  m_results.cancel();
}

bool AnalyserManager::isAnalysingFiles() {
  return m_results.isRunning();
}

void AnalyserManager::readINI(const QString& filename){
  if (!QFileInfo(filename).exists())
    return;
  
  clear();
  
  QSettings s(filename, QSettings::IniFormat);
  s.setIniCodec("UTF-8");

  s.beginGroup(QLatin1String("files"));
  QStringList files = s.childGroups();
  while (files.size()) {
    s.beginGroup(files.takeFirst()); // Group: FILE
    //stack.append(current->getChilds());
    FileResult* fr = new FileResult(s.value(QLatin1String("filename")).toString());
    fr->setEnabled(s.value(QLatin1String("enabled")).toBool());
    fr->setLines(s.value(QLatin1String("lines")).toInt());
    fr->setBgColor(s.value(QLatin1String("bgcolor"), m_fileBackgroundColor).value<QColor>());
    const int len = s.beginReadArray(QLatin1String("concerns"));
    for (int i=0;i<len;++i) {
      s.setArrayIndex(i);
      Concern c;
      c.name = s.value(QLatin1String("name")).toString();
      c.lineStart = s.value(QLatin1String("start")).toInt();
      c.lineEnd = s.value(QLatin1String("end")).toInt();
      fr->addConcern(c);
    }
    m_files.append(fr);
    s.endArray(); // EndGroup: concerns
    s.endGroup(); // EndGroup: File
  }
  s.endGroup(); // EndGroup: Files

  {
      const int len = s.beginReadArray(QLatin1String("ifdefs"));
      for (int i=0;i<len;++i) {
        s.setArrayIndex(i);
        ifdefStruct c;
        c.name = s.value(QLatin1String("name")).toString();
        c.enabled = s.value(QLatin1String("enabled")).toBool();
        c.color = s.value(QLatin1String("color"), m_fileBackgroundColor).value<QColor>();
        m_ifdefs.insert(c.name, c);
      }
      s.endArray(); // EndGroup: ifdefs
  }

  postAnalyse();

  Q_EMIT progressValueChanged(m_files.size());
  Q_EMIT finishedCompletly();

}


void AnalyserManager::setStartDirectory(const QDir& startdir, bool recursivly){
  m_startdir = startdir;
  m_recursivly = recursivly;
}

void AnalyserManager::setFileFilter(const QStringList& filter){
  m_filters = filter;
}

QStringList AnalyserManager::getFileFilter() const {
    return m_filters;
}

QList<FileResult*> AnalyserManager::files() const {
    return m_files;
}

void AnalyserManager::setFileEnable(int v, bool enabled) {
    m_files[v]->setEnabled(enabled);
}

void AnalyserManager::setHideFiles(bool notRelatedConditionalComments, bool noConditionalComments) {
    m_HidenotRelatedConditionalComments = notRelatedConditionalComments;
    m_HidenoConditionalComments = noConditionalComments;
}

void AnalyserManager::setVerticalOutput(bool b) {
    m_verticalOutput = b;
}

void AnalyserManager::setOutputWithLinebreak(bool b, int count) {
    m_outputWithLinebreak = b;
    m_outputWithLinebreakCount = count;
}

void AnalyserManager::setFileSizeCorrelatesRectangle(bool b) {
    m_fileSizeCorrelatesRectangle = b;
}

void AnalyserManager::setSortFiles(bool b) {
    m_sortFilesSize=b;
}

void AnalyserManager::setFileBackgroundColor(const QString& filename, QColor c) {
    for (int i=0;i< m_files.size();++i) {
        if (QFileInfo(m_files[i]->getFilename()).fileName()==filename) {
            m_files[i]->setBgColor(c);
            break;
        }
    }
}

QColor AnalyserManager::defaultFileBackgroundColor() const {
    return m_fileBackgroundColor;
}

void AnalyserManager::setFileItemDimensionLimit(int min, int max, int width) {
    m_min = min;
    m_max = max;
    m_width = width;
}

void AnalyserManager::startAnalysingFiles(){
  clear();
  
  QStringList dirstack;
  QStringList allfiles;
  
  dirstack.append(m_startdir.absolutePath());
  while (dirstack.size()) {
    QDir cdir(dirstack.takeFirst());
    QStringList files = cdir.entryList(m_filters, QDir::NoDotAndDotDot|QDir::Files);
    for(int i=0;i<files.size();++i) {
      allfiles.append(cdir.absoluteFilePath(files[i]));
    }

    if (!m_recursivly)
        break;

    QStringList dirs = cdir.entryList(QDir::NoDotAndDotDot|QDir::Dirs);
    for(int i=0;i<dirs.size();++i) {
      dirs[i] = cdir.absoluteFilePath(dirs[i]);
    }
    dirstack.append(dirs);
  }
  m_results = QtConcurrent::mapped(allfiles, CFileAnalyser::analyseFile);
  m_fileresultFuture.setFuture(m_results);
}

void AnalyserManager::clear(){
  if (m_results.isRunning())
    return;
  
  qDeleteAll(m_files);
  m_files.clear();
  m_results.cancel();
  m_results.waitForFinished();
  m_results = QFuture<FileResult*>();
}

QByteArray AnalyserManager::svgFileConcernRect(int x, int y, int barwidth, int barheight, FileResult* current, bool verticalOutput) {
    QByteArray t;

    QColor bgc = current->bgColor();
    t += "<rect x=\""+QByteArray::number(x)+"\" y=\""+QByteArray::number(y)+
            "\" width=\""+QByteArray::number(barwidth)+"\" height=\""+QByteArray::number(barheight)+
            "\" style=\"fill:rgb("+QByteArray::number(bgc.red())+","+
            QByteArray::number(bgc.green())+","+QByteArray::number(bgc.blue())+");"+
            "fill-opacity:"+QByteArray::number(m_fileBackgroundColor.alphaF())+
            ";stroke-width:1;stroke:rgb(0,0,0)\" />\n";

    // concerns
    for (int i=0,len=current->foundConcerns().size();i<len;++i) {
        Concern c = current->foundConcerns()[i];
        QMap<QString, ifdefStruct>::iterator it= m_ifdefs.find(c.name);
        if (it==m_ifdefs.end() || !it->enabled)
          continue;
        QColor featureColor = it->color;
        int concern_y, concern_x, concern_height, concern_width;
        if (verticalOutput) {
            concern_y = y;
            concern_x = x+c.lineStart*barwidth/current->getLines();
            concern_height = barheight;
            concern_width = (c.lineEnd-c.lineStart)*barwidth/current->getLines();
            if (concern_width==0)
                concern_width = 1;
        } else {
            concern_y = y+c.lineStart*barheight/current->getLines();
            concern_x = x;
            concern_height = (c.lineEnd-c.lineStart)*barheight/current->getLines();
            concern_width = barwidth;
            if (concern_height==0)
                concern_height = 1;
        }
        t += "\t<rect x=\""+QByteArray::number(concern_x)+"\" y=\""+QByteArray::number(concern_y)+
                "\" width=\""+QByteArray::number(concern_width)+"\" height=\""+QByteArray::number(concern_height)+
                "\" style=\"fill:rgb("+QByteArray::number(featureColor.red())+","+
                QByteArray::number(featureColor.green())+","+QByteArray::number(featureColor.blue())+");opacity:"+QByteArray::number(featureColor.alphaF())+"\">\n\t\t<title>";
        t += c.name.toUtf8().replace('>',"").replace('<',"").replace('&',"");
        t += "</title>\n\t</rect>\n";
    }

    return t;
}

QByteArray AnalyserManager::svgFilenameText(int x, int y, const QString& filename, bool verticalText) {
    if (verticalText)
    return "<text x=\""+QByteArray::number(x)+
            "\" y=\""+QByteArray::number(y)+
            "\" style=\"font-size:30;writing-mode:tb;\" fill=\"black\">"+filename.toUtf8()+"</text>\n";
    else
        return "<text x=\""+QByteArray::number(x)+
                "\" y=\""+QByteArray::number(y)+
                "\" style=\"font-size:30;\" fill=\"black\">"+filename.toUtf8()+"</text>\n";
}

bool AnalyserManager::sortSize(const FileResult* r1, const FileResult* r2) {
    return r1->getLines() < r2->getLines();
}

QPair<QByteArray, QSize> AnalyserManager::generateSVG()
{
    QFont f;
    f.setPointSize(20);
    QFontMetrics fm = QFontMetrics(f);
  QList<FileResult*> stack;
  stack.append(m_files);
  if (m_sortFilesSize) {
      qSort(stack.begin(), stack.end(), AnalyserManager::sortSize);
  }
  
  const int maxbarlength = m_max;
  const int barwidth = m_width;
  const int textoffset = 5;
  const int spaceoffset = 10;
  int x = 0;
  int y = 5;

  int maxwidth = 0;
  int maxheight = 0;
  int maxrowheight = 0;
  int maxcolumnwidth = 0;

  int fileCounter = 0;
  QByteArray svgmain;
  while (stack.size()) {
    QByteArray svgfileoutput;
    FileResult* current = stack.takeFirst();
    // file active?
    if (!current->getEnabled())
        continue;
    // any active concern in it?
    if (m_HidenoConditionalComments && current->foundConcerns().isEmpty()) {
        continue;
    }
    if (m_HidenotRelatedConditionalComments) {
        bool skip = true;
        for (int i=0,len=current->foundConcerns().size();i<len;++i) {
            Concern c = current->foundConcerns()[i];
            QMap<QString, ifdefStruct>::iterator it= m_ifdefs.find(c.name);
            if (it==m_ifdefs.end() || !it->enabled)
                continue;

            // We found at least one concern in this file that is enabled
            skip = false;
            break;
        }
        if (skip)
            continue;
    }

    const QString filename = QFileInfo(current->getFilename()).fileName();
    int barlength = m_fileSizeCorrelatesRectangle ? maxbarlength*current->normalizedSize() : maxbarlength;
    if (barlength<m_min) barlength = m_min;

    // begin new line if option is set and enough files aready visualized
    if (m_outputWithLinebreak && fileCounter>=m_outputWithLinebreakCount) {
        fileCounter=0;
        if (m_verticalOutput) {
            maxwidth += maxcolumnwidth;
            maxcolumnwidth = 0;
            y = 0;
            x = maxwidth + spaceoffset;
        } else {
            maxheight += maxrowheight;
            maxrowheight = 0;
            x = 0;
            y = maxheight + spaceoffset;
        }
    }

    if (m_verticalOutput) {
        svgfileoutput += svgFileConcernRect(x, y, barlength, barwidth, current, true);
        svgfileoutput += svgFilenameText(x+barlength+textoffset, y+barwidth/2, filename, false);

        maxheight = qMax(maxheight, y+barwidth);
        maxcolumnwidth = qMax(maxcolumnwidth, barlength+fm.width(filename)+2);
        y += spaceoffset + barwidth;

    } else {
        svgfileoutput += svgFileConcernRect(x, y, barwidth, barlength, current, false);
        svgfileoutput += svgFilenameText(x+barwidth/2, y+barlength+textoffset, filename, true);

        maxwidth = qMax(maxwidth, x+barwidth);
        maxrowheight = qMax(maxrowheight, barlength+fm.width(filename)+2);
        x += spaceoffset + barwidth;
    }

      ++fileCounter;
      svgmain += svgfileoutput;
  }

  // calculation
  if (m_verticalOutput) {
    maxwidth += maxcolumnwidth;
  } else {
      maxheight += maxrowheight;
  }


  const QByteArray header = "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
  const QByteArray footer = "</svg>";

  return QPair<QByteArray, QSize>(header + svgmain + footer, QSize(maxwidth, maxheight));
}

QByteArray AnalyserManager::getSVG() {
    return m_resultSVG;
}

QSize AnalyserManager::getSVGDimension() {
    return m_resultSVGDimension;
}

QPair<QByteArray, QSize> _helpergenerateSVGAsync(AnalyserManager* am) {
    return am->generateSVG();
}

void AnalyserManager::generateSVGAsync() {
    m_resultSVGFuture = QtConcurrent::run(_helpergenerateSVGAsync, this);
    m_resultSVGFutureWatcher.setFuture(m_resultSVGFuture);
}

void AnalyserManager::generateSVGFinished() {
    QPair<QByteArray, QSize> result = m_resultSVGFuture.result();
    m_resultSVG = result.first;
    m_resultSVGDimension = result.second;
    m_resultSVGFuture = QFuture< QPair<QByteArray, QSize> >();
    Q_EMIT svgGenerated();
}

void AnalyserManager::generateSVG(const QString& filename){
  QFile f(filename);
  f.open(QFile::WriteOnly|QFile::Truncate);
  f.write(m_resultSVG);
  f.close();
}

void AnalyserManager::generateINI(const QString& filename){
  QFile(filename).remove();
  QSettings s(filename, QSettings::IniFormat);
  s.setIniCodec("UTF-8");

  s.beginGroup(QLatin1String("files"));
  QList<FileResult*> stack;
  stack.append(m_files);
  
  while (stack.size()) {
    FileResult* current = stack.takeFirst();
    //stack.append(current->getChilds());
    s.beginGroup(current->getFilename().replace(QLatin1String("/"), QLatin1String("_")));
    s.setValue(QLatin1String("filename"), current->getFilename());
    s.setValue(QLatin1String("enabled"), current->getEnabled());
    s.setValue(QLatin1String("bgcolor"), current->bgColor());
    s.setValue(QLatin1String("lines"), current->getLines());
    s.beginWriteArray(QLatin1String("concerns"));
    for (int i=0,len=current->foundConcerns().size();i<len;++i) {
      Concern c = current->foundConcerns()[i];
      s.setArrayIndex(i);
      s.setValue(QLatin1String("name"), c.name);
      s.setValue(QLatin1String("start"), c.lineStart);
      s.setValue(QLatin1String("end"), c.lineEnd);
    }
    s.endArray();
    s.endGroup();
  }
  s.endGroup();

  s.beginWriteArray(QLatin1String("ifdefs"));
  QMap<QString, ifdefStruct>::Iterator it = m_ifdefs.begin();
  int i = 0;
  for (;it!=m_ifdefs.end();++it) {
    ifdefStruct c = *it;
    s.setArrayIndex(i++);
    s.setValue(QLatin1String("name"), c.name);
    s.setValue(QLatin1String("enabled"), c.enabled);
    s.setValue(QLatin1String("color"), c.color);
  }
  s.endArray();
}

QList<AnalyserManager::ifdefStruct> AnalyserManager::ifdefs() {
    return m_ifdefs.values();
}

QColor AnalyserManager::IFDEFColor(const QString& ifdefname) const {
    return m_ifdefs[ifdefname].color;
}

void AnalyserManager::setIFDEFColor(const QString& ifdefname, QColor c) {
    m_ifdefs[ifdefname].color = c;
}

void AnalyserManager::setIFDEFEnabled(const QString& ifdefname, bool enabled) {
    m_ifdefs[ifdefname].enabled = enabled;
}

void AnalyserManager::postAnalyse() {
    float biggest = 0;
    for(QList<FileResult*>::const_iterator i = m_files.begin();i!=m_files.end();++i) {
        FileResult* r = *i;
        if (r->getLines()>biggest)
            biggest = r->getLines();
    }
    for(QList<FileResult*>::const_iterator i = m_files.begin();i!=m_files.end();++i) {
        FileResult* r = *i;
        // normalize
        r->setNormalizedSize(r->getLines()/biggest);
    }
}

void AnalyserManager::analyseCanceled() {
    Q_EMIT finishedCompletly(true);
}

void AnalyserManager::allFilesAnalysed() {
  for(QFuture<FileResult*>::const_iterator i = m_results.begin();i!=m_results.end();++i) {
      FileResult* r = *i;
        m_files.append(r);
       r->setBgColor(m_fileBackgroundColor);
  }
  QSet<QString> concern_names;
  for(QList<FileResult*>::const_iterator i = m_files.begin();i!=m_files.end();++i) {
        FileResult* r = *i;

        // determine concerns of this file
        for (int i=0,len=r->foundConcerns().size();i<len;++i) {
              Concern c = r->foundConcerns()[i];
              if (c.name.size())
                concern_names.insert(c.name);
        }
  }

  // Create ifdefs
    m_ifdefs.clear();
    for(QSet<QString>::const_iterator i = concern_names.begin();i!=concern_names.end();++i) {
        ifdefStruct s;
        s.name = *i;
        s.enabled = true;
        s.color = m_fileMarkDefaultColor;
        m_ifdefs.insert(*i, s);
    }

  postAnalyse();

  Q_EMIT finishedCompletly();
}
