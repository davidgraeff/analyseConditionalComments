#pragma once
#include <qstring.h>
#include <qrunnable.h>
#include "fileresult.h"

class CFileAnalyser
{
public:
  static FileResult* analyseFile(const QString& file);
};
