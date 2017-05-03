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
bool IsAbsolute(const TWideString& rkDirPath);
bool IsRelative(const TWideString& rkDirPath);
bool IsEmpty(const TWideString& rkDirPath);
bool MakeDirectory(const TWideString& rkNewDir);
bool CopyFile(const TWideString& rkOrigPath, const TWideString& rkNewPath);
bool CopyDirectory(const TWideString& rkOrigPath, const TWideString& rkNewPath);
bool MoveFile(const TWideString& rkOldPath, const TWideString& rkNewPath);
bool MoveDirectory(const TWideString& rkOldPath, const TWideString& rkNewPath);
bool DeleteFile(const TWideString& rkFilePath);
bool DeleteDirectory(const TWideString& rkDirPath, bool FailIfNotEmpty); // This is an extremely destructive function, be careful using it!
bool ClearDirectory(const TWideString& rkDirPath);  // This is an extremely destructive function, be careful using it!
u64 FileSize(const TWideString& rkFilePath);
u64 LastModifiedTime(const TWideString& rkFilePath);
TWideString WorkingDirectory();
TWideString MakeAbsolute(TWideString Path);
TWideString MakeRelative(const TWideString& rkPath, const TWideString& rkRelativeTo = WorkingDirectory());
TWideString SimplifyRelativePath(const TWideString& rkPath);
TWideString SanitizeName(TWideString Name, bool Directory, bool RootDir = false);
TWideString SanitizePath(TWideString Path, bool Directory);
bool IsValidName(const TWideString& rkName, bool Directory, bool RootDir = false);
bool IsValidPath(const TWideString& rkPath, bool Directory);
void GetDirectoryContents(TWideString DirPath, TWideStringList& rOut, bool Recursive = true, bool IncludeFiles = true, bool IncludeDirs = true);
TWideString FindFileExtension(const TWideString& rkDir, const TWideString& rkName);

}

#endif // FILEUTIL

