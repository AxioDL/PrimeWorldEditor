#ifndef FILEUTIL
#define FILEUTIL

#include "Flags.h"
#include "TString.h"

namespace FileUtil
{

bool Exists(const TString& rkFilePath);
bool IsRoot(const TString& rkPath);
bool IsFile(const TString& rkFilePath);
bool IsDirectory(const TString& rkDirPath);
bool IsAbsolute(const TString& rkDirPath);
bool IsRelative(const TString& rkDirPath);
bool IsEmpty(const TString& rkDirPath);
bool MakeDirectory(const TString& rkNewDir);
bool CopyFile(const TString& rkOrigPath, const TString& rkNewPath);
bool CopyDirectory(const TString& rkOrigPath, const TString& rkNewPath);
bool MoveFile(const TString& rkOldPath, const TString& rkNewPath);
bool MoveDirectory(const TString& rkOldPath, const TString& rkNewPath);
bool DeleteFile(const TString& rkFilePath);
bool DeleteDirectory(const TString& rkDirPath, bool FailIfNotEmpty); // This is an extremely destructive function, be careful using it!
bool ClearDirectory(const TString& rkDirPath);  // This is an extremely destructive function, be careful using it!
u64 FileSize(const TString& rkFilePath);
u64 LastModifiedTime(const TString& rkFilePath);
TString WorkingDirectory();
TString MakeAbsolute(TString Path);
TString MakeRelative(const TString& rkPath, const TString& rkRelativeTo = WorkingDirectory());
TString SimplifyRelativePath(const TString& rkPath);
u32 MaxFileNameLength();
TString SanitizeName(TString Name, bool Directory, bool RootDir = false);
TString SanitizePath(TString Path, bool Directory);
bool IsValidFileNameCharacter(char Chr);
bool IsValidName(const TString& rkName, bool Directory, bool RootDir = false);
bool IsValidPath(const TString& rkPath, bool Directory);
void GetDirectoryContents(TString DirPath, TStringList& rOut, bool Recursive = true, bool IncludeFiles = true, bool IncludeDirs = true);
TString FindFileExtension(const TString& rkDir, const TString& rkName);
bool LoadFileToString(const TString& rkFilePath, TString& rOut);

}

#endif // FILEUTIL

