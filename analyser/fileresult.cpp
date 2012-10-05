#include "fileresult.h"
#include <QDir>

FileResult::FileResult(const QString& file) : m_filename(file), m_enabled(true), m_normalizedSize(1.0)
{

}

FileResult::~FileResult()
{

}

