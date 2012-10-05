#pragma once

#include <QObject>
#include <QDir>
#include <qfuture.h>
#include <qthread.h>
#include <qfuturewatcher.h>
#include <QColor>

#include "analyser/fileresult.h"

class AnalyserManager : public QFutureWatcher<FileResult*>
{
    Q_OBJECT
public:
  /**
   * Scan all *.h,*.cpp,*.c,*.hpp files in startDir recursivly and return an svg with the output
   */
    explicit AnalyserManager(QObject *parent = 0);
    virtual ~AnalyserManager();
    QColor fileMarkDefaultColor() const;
    void readINI(const QString& filename);

    void setStartDirectory(const QDir& startdir, bool recursivly);
    void setFileFilter(const QStringList& filter);
    QStringList getFileFilter() const;

    QStringList foundFiles() const;
    void setFileEnable(int v, bool enabled);
    void setHideNotRelatedFiles(bool b);

    void setVerticalOutput(bool b);
    void setOutputWithLinebreak(bool b, int count);

    void setFileSizeCorrelatesRectangle(bool b);
    void setFileBackgroundColor(QColor c);
    QColor fileBackgroundColor() const;

    struct ifdefStruct {
        bool enabled;
        QString name;
        QColor color;
    };

    QList<ifdefStruct> ifdefs();
    QColor IFDEFColor(const QString &ifdefname) const;
    void setIFDEFColor(const QString &ifdefname, QColor c);
    void setIFDEFEnabled(const QString &ifdefname, bool enabled);

    void setFileItemDimensionLimit(int min, int max, int width);

    /**
     * Clear all results
     */
    void clear();
    /**
     * Abort analysing but collected results remain
     */
    void cancel();
    
    void start();
    
    
    /**
     * Available after start()
     * Output
     */
    void generateSVG(const QString& filename);
    QByteArray generateSVG();
    void generateINI(const QString& filename);

private:
    QFuture<FileResult*> m_results;
    QList<FileResult*> m_files;
    QDir m_startdir;
    bool m_recursivly;
    bool m_hideNotRelatedFiles;
    bool m_verticalOutput;
    bool m_outputWithLinebreak;
    int m_outputWithLinebreakCount;
    bool m_fileSizeCorrelatesRectangle;
    QColor m_fileBackgroundColor;
    QColor m_fileMarkDefaultColor;
    int m_min;
    int m_max;
    int m_width;
    QMap<QString, ifdefStruct> m_ifdefs;
    QStringList m_filters;
    bool m_cancel;
    void postAnalyse();
    QByteArray svgFileConcernRect(int x, int y, int barwidth, int barheight, FileResult *current, bool verticalOutput);
    QByteArray svgFilenameText(int x, int y, const QString& filename, bool verticalText);
Q_SIGNALS:
    void finishedCompletly(bool aborted = false);
public Q_SLOTS:
    void allFilesAnalysed();
};
