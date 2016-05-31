#include "FileUtil.h"
#include "AssertMacro.h"
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

// These are mostly just wrappers around boost::filesystem functions.
using namespace boost::filesystem;

namespace FileUtil
{

bool Exists(const TWideString &rkFilePath)
{
    return exists(*rkFilePath);
}

bool IsRoot(const TWideString& rkPath)
{
    // todo: verify that this is actually a good way of checking for root
    TWideString AbsPath = MakeAbsolute(rkPath);
    TWideStringList Split = AbsPath.Split(L"\\/");
    return (Split.size() <= 1);
}

bool IsFile(const TWideString& rkFilePath)
{
    return is_regular_file(*rkFilePath);
}

bool IsDirectory(const TWideString& rkDirPath)
{
    return is_directory(*rkDirPath);
}

bool CreateDirectory(const TWideString& rkNewDir)
{
    return create_directories(*rkNewDir);
}

bool CopyFile(const TWideString& rkOrigPath, const TWideString& rkNewPath)
{
    boost::system::error_code Error;
    copy(*rkOrigPath, *rkNewPath, Error);
    return (Error == boost::system::errc::success);
}

bool CopyDirectory(const TWideString& rkOrigPath, const TWideString& rkNewPath)
{
    boost::system::error_code Error;
    copy_directory(*rkOrigPath, *rkNewPath, Error);
    return (Error == boost::system::errc::success);
}

bool MoveFile(const TWideString& rkOldPath, const TWideString& rkNewPath)
{
    if (CopyFile(rkOldPath, rkNewPath))
        return DeleteFile(rkOldPath);
    else
        return false;
}

bool MoveDirectory(const TWideString& rkOldPath, const TWideString& rkNewPath)
{
    if (CopyDirectory(rkOldPath, rkNewPath))
        return DeleteDirectory(rkOldPath);
    else
        return false;
}

bool DeleteFile(const TWideString& rkFilePath)
{
    if (!IsFile(rkFilePath)) return false;
    return remove(*rkFilePath) == 1;
}

bool DeleteDirectory(const TWideString& rkDirPath)
{
    // This is an extremely destructive function, be careful using it!
    if (!IsDirectory(rkDirPath)) return false;

    // Sanity check - don't delete root
    bool Root = IsRoot(rkDirPath);

    if (Root)
    {
        ASSERT(false);
        Log::Error("Attempted to delete root directory!");
        return false;
    }

    boost::system::error_code Error;
    remove_all(*rkDirPath, Error);
    return (Error == boost::system::errc::success);
}

bool ClearDirectory(const TWideString& rkDirPath)
{
    // This is an extremely destructive function, be careful using it!
    if (!IsDirectory(rkDirPath)) return false;

    // Sanity check - don't clear root
    bool Root = IsRoot(rkDirPath);

    if (Root)
    {
        ASSERT(false);
        Log::Error("Attempted to clear root directory!");
        return false;
    }

    // Delete directory contents
    TWideStringList DirContents;
    GetDirectoryContents(rkDirPath, DirContents, false);

    for (auto It = DirContents.begin(); It != DirContents.end(); It++)
    {
        bool Success = false;

        if (IsFile(*It))
            Success = DeleteFile(*It);
        else if (IsDirectory(*It))
            Success = DeleteDirectory(*It);

        if (!Success)
            Log::Error("Failed to delete filesystem object: " + TWideString(*It).ToUTF8());
    }

    return true;
}

u64 FileSize(const TWideString &rkFilePath)
{
    return (u64) (Exists(*rkFilePath) ? file_size(*rkFilePath) : -1);
}

u64 LastModifiedTime(const TWideString& rkFilePath)
{
    return (u64) last_write_time(*rkFilePath);
}

TWideString WorkingDirectory()
{
    return boost::filesystem::current_path().string();
}

TWideString MakeAbsolute(TWideString Path)
{
    if (!boost::filesystem::path(*Path).has_root_name())
        Path = WorkingDirectory() + L"\\" + Path;

    TWideStringList Components = Path.Split(L"/\\");
    TWideStringList::iterator Prev;

    for (TWideStringList::iterator Iter = Components.begin(); Iter != Components.end(); Iter++)
    {
        if (*Iter == L".")
            Iter = Components.erase(Iter);
        else if (*Iter == L"..")
            Iter = std::prev(Components.erase(Prev, std::next(Iter)));

        Prev = Iter;
    }

    TWideString Out;
    for (auto it = Components.begin(); it != Components.end(); it++)
        Out += *it + L"\\";

    return Out;
}

TWideString MakeRelative(const TWideString& rkPath, const TWideString& rkRelativeTo /*= WorkingDirectory()*/)
{
    TWideString AbsPath = MakeAbsolute(rkPath);
    TWideString AbsRelTo = MakeAbsolute(rkRelativeTo);
    TWideStringList PathComponents = AbsPath.Split(L"/\\");
    TWideStringList RelToComponents = AbsRelTo.Split(L"/\\");

    // Find furthest common parent
    TWideStringList::iterator PathIter = PathComponents.begin();
    TWideStringList::iterator RelToIter = RelToComponents.begin();

    for (; PathIter != PathComponents.end() && RelToIter != RelToComponents.end(); PathIter++, RelToIter++)
    {
        if (*PathIter != *RelToIter)
            break;
    }

    // If there's no common components, then we can't construct a relative path...
    if (PathIter == PathComponents.begin())
        return AbsPath;

    // Construct output path
    TWideString Out;

    for (; RelToIter != RelToComponents.end(); RelToIter++)
        Out += L"..\\";

    for (; PathIter != PathComponents.end(); PathIter++)
        Out += *PathIter + L"\\";

    return Out;
}

void GetDirectoryContents(TWideString DirPath, TWideStringList& rOut, bool Recursive /*= true*/, bool IncludeFiles /*= true*/, bool IncludeDirs /*= true*/)
{
    if (IsDirectory(DirPath))
    {
        DirPath.Replace(L"/", L"\\");
        bool IncludeAll = IncludeFiles && IncludeDirs;

        auto AddFileLambda = [IncludeFiles, IncludeDirs, IncludeAll, &rOut](std::wstring Path) -> void {
            bool ShouldAddFile = IncludeAll || (IncludeFiles && IsFile(Path)) || (IncludeDirs && IsDirectory(Path));

            if (ShouldAddFile)
                rOut.push_back(Path);
        };

        if (Recursive)
        {
            for (recursive_directory_iterator It(*DirPath); It != recursive_directory_iterator(); ++It)
            {
#ifdef _WIN32
                AddFileLambda( It->path().native() );
#else
                AddFileLambda( TString(It->path().native()).ToUTF16().ToStdString() );
#endif
            }
        }

        else
        {
            for (directory_iterator It(*DirPath); It != directory_iterator(); ++It)
            {
#ifdef _WIN32
                AddFileLambda( It->path().native() );
#else
                AddFileLambda( TString(It->path().native()).ToUTF16().ToStdString() );
#endif
            }
        }
    }
}

}
