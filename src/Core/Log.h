#ifndef INFO
#define INFO

#include <Common/TString.h>

namespace Log
{

void Write(const TString& rkMessage);
void Error(const TString& rkMessage);
void Warning(const TString& rkMessage);
void FileWrite(const TString& rkFilename, const TString& rkMessage);
void FileWrite(const TString& rkFilename, unsigned long Offset, const TString& rkMessage);
void FileError(const TString& rkFilename, const TString& rkMessage);
void FileError(const TString& rkFilename, unsigned long Offset, const TString& rkMessage);
void FileWarning(const TString& rkFilename, const TString& rkMessage);
void FileWarning(const TString& rkFilename, unsigned long Offset, const TString& rkMessage);
const TStringList& GetErrorLog();
void ClearErrorLog();

}

#endif // INFO

