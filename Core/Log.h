#ifndef INFO
#define INFO

#include <Common/TString.h>

namespace Log
{

void Write(const TString& message);
void Error(const TString& message);
void Warning(const TString& message);
void FileWrite(const TString& filename, const TString& message);
void FileWrite(const TString& filename, unsigned long offset, const TString& message);
void FileError(const TString& filename, const TString& message);
void FileError(const TString& filename, unsigned long offset, const TString& message);
void FileWarning(const TString& filename, const TString& message);
void FileWarning(const TString& filename, unsigned long offset, const TString& message);

}

#endif // INFO

