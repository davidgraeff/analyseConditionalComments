#pragma once
#include <qlist.h>
#include <QSet>
#include <qstringlist.h>
#include <QColor>

class Concern
{
public:
  int lineStart;
  int lineEnd;
  QString name;
};

class FileResult
{
public:
    FileResult(const QString& file);
    virtual ~FileResult();

    const QList<Concern>& foundConcerns() const {return m_concerns;}
    QString getFilename() const {return m_filename;}
    int getLines() const { return m_maxlines; }
    void setLines(int v) { m_maxlines = v; }
    void addConcern(Concern c) {m_concerns.append(c); }
    void removeConcern(int index) {m_concerns.removeAt(index); }
    void setEnabled(bool e) { m_enabled = e; }
    bool getEnabled() { return m_enabled; }
    void setNormalizedSize(float f) { m_normalizedSize = f; }
    float normalizedSize() { return m_normalizedSize; }
    QColor bgColor() { return m_bgColor; }
    void setBgColor(QColor c) { m_bgColor = c;}
private:
    QList<Concern> m_concerns;
    QString m_filename;
    int m_maxlines;
    bool m_enabled;
    float m_normalizedSize;
    QColor m_bgColor;
};
