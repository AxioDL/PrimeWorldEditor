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
    // todo: is this actually a good/multiplatform way of checking for root?
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

bool IsAbsolute(const TWideString& rkDirPath)
{
    return boost::filesystem::path(*rkDirPath).is_absolute();
}

bool IsRelative(const TWideString& rkDirPath)
{
    return !boost::filesystem::path(*rkDirPath).is_relative();
}

bool CreateDirectory(const TWideString& rkNewDir)
{
    if (!IsValidPath(rkNewDir, true))
    {
        Log::Error("Unable to create directory because name contains illegal characters: " + rkNewDir.ToUTF8());
        return false;
    }

    return create_directories(*rkNewDir);
}

bool CopyFile(const TWideString& rkOrigPath, const TWideString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, false))
    {
        Log::Error("Unable to copy file because destination name contains illegal characters: " + rkNewPath.ToUTF8());
        return false;
    }

    boost::system::error_code Error;
    copy(*rkOrigPath, *rkNewPath, Error);
    return (Error == boost::system::errc::success);
}

bool CopyDirectory(const TWideString& rkOrigPath, const TWideString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, true))
    {
        Log::Error("Unable to copy directory because destination name contains illegal characters: " + rkNewPath.ToUTF8());
        return false;
    }

    boost::system::error_code Error;
    copy_directory(*rkOrigPath, *rkNewPath, Error);
    return (Error == boost::system::errc::success);
}

bool MoveFile(const TWideString& rkOldPath, const TWideString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, false))
    {
        Log::Error("Unable to move file because destination name contains illegal characters: " + rkNewPath.ToUTF8());
        return false;
    }

    if (CopyFile(rkOldPath, rkNewPath))
        return DeleteFile(rkOldPath);
    else
        return false;
}

bool MoveDirectory(const TWideString& rkOldPath, const TWideString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, true))
    {
        Log::Error("Unable to move directory because destination name contains illegal characters: " + rkNewPath.ToUTF8());
        return false;
    }

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

static const wchar_t gskIllegalNameChars[] = {
    L'<', L'>', L'\"', L'/', L'\\', L'|', L'?', L'*'
};

TWideString SanitizeName(TWideString Name, bool Directory, bool RootDir /*= false*/)
{
    // Windows only atm
    if (Directory && (Name == L"." || Name == L".."))
        return Name;

    // Remove illegal characters from path
    u32 NumIllegalChars = sizeof(gskIllegalNameChars) / sizeof(wchar_t);

    for (u32 iChr = 0; iChr < Name.Size(); iChr++)
    {
        wchar_t Chr = Name[iChr];
        bool Remove = false;

        if (Chr >= 0 && Chr <= 31)
            Remove = true;

        // For root, allow colon only as the last character of the name
        else if (Chr == L':' && (!RootDir || iChr != Name.Size() - 1))
            Remove = true;

        else
        {
            for (u32 iBan = 0; iBan < NumIllegalChars; iBan++)
            {
                if (Chr == gskIllegalNameChars[iBan])
                {
                    Remove = true;
                    break;
                }
            }
        }

        if (Remove)
        {
            Name.Remove(iChr, 1);
            iChr--;
        }
    }

    // For directories, space and dot are not allowed at the end of the path
    if (Directory)
    {
        u32 ChopNum = 0;

        for (int iChr = (int) Name.Size() - 1; iChr >= 0; iChr--)
        {
            wchar_t Chr = Name[iChr];

            if (Chr == L' ' || Chr == L'.')
                ChopNum++;
            else
                break;
        }

        if (ChopNum > 0) Name = Name.ChopBack(ChopNum);
    }

    return Name;
}

TWideString SanitizePath(TWideString Path, bool Directory)
{
    TWideStringList Components = Path.Split(L"\\/");
    u32 CompIdx = 0;
    Path = L"";

    for (auto It = Components.begin(); It != Components.end(); It++)
    {
        TWideString Comp = *It;
        bool IsDir = Directory || CompIdx < Components.size() - 1;
        bool IsRoot = CompIdx == 0;
        SanitizeName(Comp, IsDir, IsRoot);

        Path += Comp;
        if (IsDir) Path += L'\\';
        CompIdx++;
    }

    return Path;
}

bool IsValidName(const TWideString& rkName, bool Directory, bool RootDir /*= false*/)
{
    // Windows only atm
    u32 NumIllegalChars = sizeof(gskIllegalNameChars) / sizeof(wchar_t);

    if (Directory && (rkName == L"." || rkName == L".."))
        return true;

    // Check for banned characters
    for (u32 iChr = 0; iChr < rkName.Size(); iChr++)
    {
        wchar_t Chr = rkName[iChr];

        if (Chr >= 0 && Chr <= 31)
            return false;

        // Allow colon only on last character of root
        if (Chr == L':' && (!RootDir || iChr != rkName.Size() - 1))
            return false;

        for (u32 iBan = 0; iBan < NumIllegalChars; iBan++)
        {
            if (Chr == gskIllegalNameChars[iBan])
                return false;
        }
    }

    if (Directory && (rkName.Back() == L' ' || rkName.Back() == L'.'))
        return false;

    return true;
}

bool IsValidPath(const TWideString& rkPath, bool Directory)
{
    // Windows only atm
    TWideStringList Components = rkPath.Split(L"\\/");

    // Validate other components
    u32 CompIdx = 0;

    for (auto It = Components.begin(); It != Components.end(); It++)
    {
        bool IsRoot = CompIdx == 0;
        bool IsDir = Directory || CompIdx < (Components.size() - 1);

        if (!IsValidName(*It, IsDir, IsRoot))
            return false;

        CompIdx++;
    }

    return true;
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

TWideString FindFileExtension(const TWideString& rkDir, const TWideString& rkName)
{
    for (directory_iterator It(*rkDir); It != directory_iterator(); ++It)
    {
        TWideString Name = It->path().filename().native();
        if (Name.GetFileName(false) == rkName) return Name.GetFileExtension();
    }

    return L"";
}

}
