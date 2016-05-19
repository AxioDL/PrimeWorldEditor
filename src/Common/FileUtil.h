#ifndef FILEUTIL
#define FILEUTIL

#include "Flags.h"
#include "TString.h"

namespace FileUtil
{

bool Exists(const TWideString& rkFilePath);
bool IsRoot(const TWideString& rkPath);
bool IsFile(const TWideString& rkFilePath);
bool IsDirectory(const TWideString& rkDirPath);
bool CreateDirectory(const TWideString& rkNewDir);
bool CopyFile(const TWideString& rkOrigPath, const TWideString& rkNewPath);
bool CopyDirectory(const TWideString& rkOrigPath, const TWideString& rkNewPath);
bool MoveFile(const TWideString& rkOldPath, const TWideString& rkNewPath);
bool MoveDirectory(const TWideString& rkOldPath, const TWideString& rkNewPath);
bool DeleteFile(const TWideString& rkFilePath);
bool DeleteDirectory(const TWideString& rkDirPath);
bool ClearDirectory(const TWideString& rkDirPath);
int FileSize(const TWideString& rkFilePath);
TWideString WorkingDirectory();
TWideString MakeAbsolute(TWideString Path);
TWideString MakeRelative(const TWideString& rkPath, const TWideString& rkRelativeTo = WorkingDirectory());
void GetDirectoryContents(TWideString DirPath, TWideStringList& rOut, bool Recursive = true, bool IncludeFiles = true, bool IncludeDirs = true);

}

#endif // FILEUTIL

