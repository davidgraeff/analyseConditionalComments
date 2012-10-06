#pragma once

#include <QObject>
#include <QDir>
#include <qfuture.h>
#include <qthread.h>
#include <qfuturewatcher.h>
#include <QColor>
#include <QSize>

#include "analyser/fileresult.h"

class AnalyserManager : public QObject
{
    Q_OBJECT
public:
    friend class MainWindow;
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

    QList<FileResult*> files() const;
    void setFileEnable(int v, bool enabled);
    void setHideFiles(bool notRelatedConditionalComments, bool noConditionalComments);

    void setVerticalOutput(bool b);
    void setOutputWithLinebreak(bool b, int count);

    void setFileSizeCorrelatesRectangle(bool b);
    void setSortFiles(bool b);
    void setFileBackgroundColor(const QString &filename, QColor c);
    QColor defaultFileBackgroundColor() const;

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
    void cancelAnalysingFiles();
    bool isAnalysingFiles();
    void startAnalysingFiles();
    
    
    /**
     * Available after start()
     * Output
     */
    void generateINI(const QString& filename);
    void generateSVG(const QString& filename);
    /// Get cached svg
    QByteArray getSVG();
    QSize getSVGDimension();
    /// Synchronous call to generate the svg
    QPair<QByteArray,QSize> generateSVG();
    /// Asynchronous call to generate the svg -> listen for signal svgGenerated()
    void generateSVGAsync();
private:
    QFutureWatcher<FileResult*> m_fileresultFuture;
    QFuture<FileResult*> m_results;
    QList<FileResult*> m_files;
    QDir m_startdir;
    bool m_recursivly;
    bool m_HidenotRelatedConditionalComments;
    bool m_HidenoConditionalComments;
    bool m_verticalOutput;
    bool m_outputWithLinebreak;
    int m_outputWithLinebreakCount;
    bool m_fileSizeCorrelatesRectangle;
    bool m_sortFilesSize;
    QColor m_fileBackgroundColor;
    QColor m_fileMarkDefaultColor;
    int m_min;
    int m_max;
    int m_width;
    QMap<QString, ifdefStruct> m_ifdefs;
    QStringList m_filters;
    bool m_cancel;
    QByteArray m_resultSVG;
    QSize m_resultSVGDimension;
    QFuture< QPair<QByteArray, QSize> > m_resultSVGFuture;
    QFutureWatcher< QPair<QByteArray, QSize> > m_resultSVGFutureWatcher;
    void postAnalyse();
    static bool sortSize(const FileResult *r1, const FileResult *r2);
    QByteArray svgFileConcernRect(int x, int y, int barwidth, int barheight, FileResult *current, bool verticalOutput);
    QByteArray svgFilenameText(int x, int y, const QString& filename, bool verticalText);
Q_SIGNALS:
    void finishedCompletly(bool aborted = false);
    void svgGenerated();
    void svgCanceled();
    void progressValueChanged(int);
private Q_SLOTS:
    void allFilesAnalysed();
    void analyseCanceled();
    void generateSVGFinished();
};
