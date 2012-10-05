#include "cfileanalyser.h"
#include <qfile.h>
#include <QDebug>
#include <qfileinfo.h>

struct ifdefEntry {
  QByteArray name;
  int startline;
  ifdefEntry(QByteArray name, int startline) {this->name = name; this->startline = startline; }
};
  
FileResult* CFileAnalyser::analyseFile(const QString& file) {
  qDebug() << "Analysiere Datei:" << file;
  FileResult* r = new FileResult(file);
  QFile f(file);
  if (!f.open(QFile::ReadOnly))
    return r;
  
  QList<ifdefEntry> ifdefStack;
  
  bool isHeader = file.right(2)==QLatin1String(".h");
  int line = 1;
  int index;
  bool newIncludeGuard = false;
  int foundConcerns = 0;
  
  while (!f.atEnd()) {
      const QByteArray l = f.readLine();
      const QByteArray ll = l.toLower();
      
      newIncludeGuard |= (bool)ll.contains("#pragma once");
      
      index = ll.indexOf("#ifdef ");
      if (index==-1)
	  index = ll.indexOf("#ifndef ");
      if (index==-1)
	  index = ll.indexOf("#if ");
      
      if (index != -1) {
	int pos = l.indexOf(" ", index)+1;
	int endpos = l.size();
	QByteArray name = l.mid(pos,endpos-pos-1).trimmed().replace(" ","");
	if (name.isEmpty()) {
	  qWarning()<<"Empty!"<<ll;
	}
	ifdefStack.append(ifdefEntry( name, line ));
      } else if (ll.contains("#endif")) {
	if (ifdefStack.isEmpty()) {
	    qWarning() << QFileInfo(file).fileName() << "#endif found!";
	} else {
	  ++foundConcerns;
	  ifdefEntry e = ifdefStack.takeLast();
//	  if (foundConcerns==1 && !newIncludeGuard && isHeader) {
//	    // Ignore include guards
//	    continue;
//	  }
	  
	  Concern c;
	  c.lineEnd = line;
	  c.lineStart = e.startline;
	  c.name = QString::fromUtf8(e.name);
	  r->addConcern(c);
	}
      }
      ++line;
  }
  f.close();
  r->setLines(line-1);

    // try with heuristic to remove include-guard #ifdef: Remove #ifdef with last #endif
  if (!newIncludeGuard && isHeader) {
      int lastConcernLine = 0, lastConcernIndex = -1;
      QList<Concern> concers = r->foundConcerns();
      for (int i=0;i<concers.size();++i) {
          if (concers[i].lineEnd > lastConcernLine) {
              lastConcernLine = concers[i].lineEnd;
              lastConcernIndex = i;
          }
      }
      if (lastConcernIndex!=-1) {
            r->removeConcern(lastConcernIndex);
      }
  }
  return r;
}
