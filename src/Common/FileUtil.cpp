#include "FileUtil.h"
#include "AssertMacro.h"
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

// These are mostly just wrappers around boost::filesystem functions.
using namespace boost::filesystem;

// Macro encapsulating a TString -> boost::filesystem::path conversion
// boost does not handle conversion from UTF-8 correctly so we need to do it manually
#define TO_PATH(String) path( *String.ToUTF16() )
#define FROM_PATH(Path) TWideString( Path.native() ).ToUTF8()

namespace FileUtil
{

bool Exists(const TString &rkFilePath)
{
    return exists( TO_PATH(rkFilePath) );
}

bool IsRoot(const TString& rkPath)
{
    // todo: is this actually a good/multiplatform way of checking for root?
    TString AbsPath = MakeAbsolute(rkPath);
    TStringList Split = AbsPath.Split("\\/");
    return (Split.size() <= 1);
}

bool IsFile(const TString& rkFilePath)
{
    return is_regular_file( TO_PATH(rkFilePath) );
}

bool IsDirectory(const TString& rkDirPath)
{
    return is_directory( TO_PATH(rkDirPath) );
}

bool IsAbsolute(const TString& rkDirPath)
{
    return boost::filesystem::path( TO_PATH(rkDirPath) ).is_absolute();
}

bool IsRelative(const TString& rkDirPath)
{
    return boost::filesystem::path( TO_PATH(rkDirPath) ).is_relative();
}

bool IsEmpty(const TString& rkDirPath)
{
    if (!IsDirectory(rkDirPath))
    {
        Log::Error("Non-directory path passed to IsEmpty(): " + rkDirPath);
        DEBUG_BREAK;
        return false;
    }

    return is_empty( TO_PATH(rkDirPath) );
}

bool MakeDirectory(const TString& rkNewDir)
{
    if (!IsValidPath(rkNewDir, true))
    {
        Log::Error("Unable to create directory because name contains illegal characters: " + rkNewDir);
        return false;
    }

    return create_directories( TO_PATH(rkNewDir) );
}

bool CopyFile(const TString& rkOrigPath, const TString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, false))
    {
        Log::Error("Unable to copy file because destination name contains illegal characters: " + rkNewPath);
        return false;
    }

    MakeDirectory(rkNewPath.GetFileDirectory());
    boost::system::error_code Error;
    copy(TO_PATH(rkOrigPath), TO_PATH(rkNewPath), Error);
    return (Error == boost::system::errc::success);
}

bool CopyDirectory(const TString& rkOrigPath, const TString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, true))
    {
        Log::Error("Unable to copy directory because destination name contains illegal characters: " + rkNewPath);
        return false;
    }

    MakeDirectory(rkNewPath.GetFileDirectory());
    boost::system::error_code Error;
    copy_directory(TO_PATH(rkOrigPath), TO_PATH(rkNewPath), Error);
    return (Error == boost::system::errc::success);
}

bool MoveFile(const TString& rkOldPath, const TString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, false))
    {
        Log::Error("Unable to move file because destination name contains illegal characters: " + rkNewPath);
        return false;
    }

    if (Exists(rkNewPath))
    {
        Log::Error("Unable to move file because there is an existing file at the destination path: " + rkNewPath);
        return false;
    }

    int Result = rename(*rkOldPath, *rkNewPath);
    return (Result == 0);
}

bool MoveDirectory(const TString& rkOldPath, const TString& rkNewPath)
{
    if (!IsValidPath(rkNewPath, true))
    {
        Log::Error("Unable to move directory because destination name contains illegal characters: " + rkNewPath);
        return false;
    }

    if (Exists(rkNewPath))
    {
        Log::Error("Unable to move directory because there is an existing directory at the destination path: " + rkNewPath);
        return false;
    }

    int Result = rename(*rkOldPath, *rkNewPath);
    return (Result == 0);
}

bool DeleteFile(const TString& rkFilePath)
{
    if (!IsFile(rkFilePath)) return false;
    return remove( TO_PATH(rkFilePath) ) == 1;
}

bool DeleteDirectory(const TString& rkDirPath, bool FailIfNotEmpty)
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

    // Check if directory is empty
    if (FailIfNotEmpty && !IsEmpty(rkDirPath))
        return false;

    // Delete directory
    boost::system::error_code Error;
    remove_all(TO_PATH(rkDirPath), Error);
    return (Error == boost::system::errc::success);
}

bool ClearDirectory(const TString& rkDirPath)
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
    TStringList DirContents;
    GetDirectoryContents(rkDirPath, DirContents, false);

    for (auto It = DirContents.begin(); It != DirContents.end(); It++)
    {
        bool Success = false;

        if (IsFile(*It))
            Success = DeleteFile(*It);
        else if (IsDirectory(*It))
            Success = DeleteDirectory(*It, false);

        if (!Success)
            Log::Error("Failed to delete filesystem object: " + TString(*It));
    }

    return true;
}

u64 FileSize(const TString &rkFilePath)
{
    return (u64) (Exists(rkFilePath) ? file_size( TO_PATH(rkFilePath) ) : -1);
}

u64 LastModifiedTime(const TString& rkFilePath)
{
    return (u64) last_write_time( TO_PATH(rkFilePath) );
}

TString WorkingDirectory()
{
    return FROM_PATH( boost::filesystem::current_path() );
}

TString MakeAbsolute(TString Path)
{
    if (!TO_PATH(Path).has_root_name())
        Path = WorkingDirectory() + "/" + Path;

    TStringList Components = Path.Split("/\\");
    TStringList::iterator Prev;

    for (TStringList::iterator Iter = Components.begin(); Iter != Components.end(); Iter++)
    {
        if (*Iter == ".")
            Iter = Components.erase(Iter);
        else if (*Iter == "..")
            Iter = std::prev(Components.erase(Prev, std::next(Iter)));

        Prev = Iter;
    }

    TString Out;
    for (auto it = Components.begin(); it != Components.end(); it++)
        Out += *it + "/";

    return Out;
}

TString MakeRelative(const TString& rkPath, const TString& rkRelativeTo /*= WorkingDirectory()*/)
{
    TString AbsPath = MakeAbsolute(rkPath);
    TString AbsRelTo = MakeAbsolute(rkRelativeTo);
    TStringList PathComponents = AbsPath.Split("/\\");
    TStringList RelToComponents = AbsRelTo.Split("/\\");

    // Find furthest common parent
    TStringList::iterator PathIter = PathComponents.begin();
    TStringList::iterator RelToIter = RelToComponents.begin();

    for (; PathIter != PathComponents.end() && RelToIter != RelToComponents.end(); PathIter++, RelToIter++)
    {
        if (*PathIter != *RelToIter)
            break;
    }

    // If there's no common components, then we can't construct a relative path...
    if (PathIter == PathComponents.begin())
        return AbsPath;

    // Construct output path
    TString Out;

    for (; RelToIter != RelToComponents.end(); RelToIter++)
        Out += "../";

    for (; PathIter != PathComponents.end(); PathIter++)
        Out += *PathIter + "/";

    // Attempt to detect if this path is a file as opposed to a directory; if so, remove trailing backslash
    if (PathComponents.back().Contains('.') && !rkPath.EndsWith('/') && !rkPath.EndsWith('\\'))
        Out = Out.ChopBack(1);

    return Out;
}

TString SimplifyRelativePath(const TString& rkPath)
{
    TStringList PathComponents = rkPath.Split("/\\");

    TStringList::iterator Iter = PathComponents.begin();
    TStringList::iterator PrevIter = Iter;

    for (auto Iter = PathComponents.begin(); Iter != PathComponents.end(); PrevIter = Iter, Iter++)
    {
        if (*Iter == ".." && *PrevIter != "..")
        {
            PrevIter = PathComponents.erase(PrevIter);
            PrevIter = PathComponents.erase(PrevIter);
            Iter = PrevIter;
            Iter--;
        }
    }

    TString Out;

    for (auto Iter = PathComponents.begin(); Iter != PathComponents.end(); Iter++)
        Out += *Iter + '/';

    return Out;
}

u32 MaxFileNameLength()
{
    return 255;
}

static const char gskIllegalNameChars[] = {
    '<', '>', '\"', '/', '\\', '|', '?', '*', ':'
};

TString SanitizeName(TString Name, bool Directory, bool RootDir /*= false*/)
{
    // Windows only atm
    if (Directory && (Name == "." || Name == ".."))
        return Name;

    // Remove illegal characters from path
    for (u32 iChr = 0; iChr < Name.Size(); iChr++)
    {
        char Chr = Name[iChr];
        bool Remove = false;

        if (Chr >= 0 && Chr <= 31)
            Remove = true;

        // Allow colon only as the last character of root
        bool IsLegalColon = (Chr == ':' && RootDir && iChr == Name.Size() - 1);

        if (!IsLegalColon && !IsValidFileNameCharacter(Chr))
            Remove = true;

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
            char Chr = Name[iChr];

            if (Chr == ' ' || Chr == '.')
                ChopNum++;
            else
                break;
        }

        if (ChopNum > 0) Name = Name.ChopBack(ChopNum);
    }

    // Remove spaces from beginning of path
    u32 NumLeadingSpaces = 0;
    while (NumLeadingSpaces < Name.Size() && Name[NumLeadingSpaces] == ' ')
        NumLeadingSpaces++;

    if (NumLeadingSpaces > 0)
        Name = Name.ChopFront(NumLeadingSpaces);

    // Ensure the name is below the character limit
    if (Name.Size() > MaxFileNameLength())
    {
        u32 ChopNum = Name.Size() - MaxFileNameLength();
        Name = Name.ChopBack(ChopNum);
    }

    return Name;
}

TString SanitizePath(TString Path, bool Directory)
{
    TStringList Components = Path.Split("\\/");
    u32 CompIdx = 0;
    Path = "";

    for (auto It = Components.begin(); It != Components.end(); It++)
    {
        TString Comp = *It;
        bool IsDir = Directory || CompIdx < Components.size() - 1;
        bool IsRoot = CompIdx == 0;
        Comp = SanitizeName(Comp, IsDir, IsRoot);

        Path += Comp;
        if (IsDir) Path += '/';
        CompIdx++;
    }

    return Path;
}

bool IsValidFileNameCharacter(char Chr)
{
    static const u32 skNumIllegalChars = sizeof(gskIllegalNameChars) / sizeof(char);

    if (Chr >= 0 && Chr <= 31)
        return false;

    for (u32 BanIdx = 0; BanIdx < skNumIllegalChars; BanIdx++)
    {
        if (Chr == gskIllegalNameChars[BanIdx])
            return false;
    }

    return true;
}

bool IsValidName(const TString& rkName, bool Directory, bool RootDir /*= false*/)
{
    // Only accounting for Windows limitations right now. However, this function should
    // ideally return the same output on all platforms to ensure projects are cross platform.
    if (rkName.IsEmpty())
        return false;

    if (rkName.Size() > MaxFileNameLength())
        return false;

    if (Directory && (rkName == "." || rkName == ".."))
        return true;

    // Check for banned characters
    for (u32 iChr = 0; iChr < rkName.Size(); iChr++)
    {
        char Chr = rkName[iChr];

        // Allow colon only as the last character of root
        bool IsLegalColon = (Chr == ':' && RootDir && iChr == rkName.Size() - 1);

        if (!IsLegalColon && !IsValidFileNameCharacter(Chr))
            return false;
    }

    if (Directory && (rkName.Back() == ' ' || rkName.Back() == '.'))
        return false;

    return true;
}

bool IsValidPath(const TString& rkPath, bool Directory)
{
    // Only accounting for Windows limitations right now. However, this function should
    // ideally return the same output on all platforms to ensure projects are cross platform.
    TStringList Components = rkPath.Split("\\/");

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

void GetDirectoryContents(TString DirPath, TStringList& rOut, bool Recursive /*= true*/, bool IncludeFiles /*= true*/, bool IncludeDirs /*= true*/)
{
    if (IsDirectory(DirPath))
    {
        DirPath.Replace("\\", "/");
        bool IncludeAll = IncludeFiles && IncludeDirs;

        auto AddFileLambda = [IncludeFiles, IncludeDirs, IncludeAll, &rOut](TString Path) -> void {
            bool ShouldAddFile = IncludeAll || (IncludeFiles && IsFile(Path)) || (IncludeDirs && IsDirectory(Path));

            if (ShouldAddFile)
                rOut.push_back(Path);
        };

        if (Recursive)
        {
            for (recursive_directory_iterator It( TO_PATH(DirPath) ); It != recursive_directory_iterator(); ++It)
            {
                AddFileLambda( FROM_PATH(It->path()) );
            }
        }

        else
        {
            for (directory_iterator It( TO_PATH(DirPath) ); It != directory_iterator(); ++It)
            {
                AddFileLambda( FROM_PATH(It->path()) );
            }
        }
    }
}

TString FindFileExtension(const TString& rkDir, const TString& rkName)
{
    for (directory_iterator It( TO_PATH(rkDir) ); It != directory_iterator(); ++It)
    {
        TString Name = FROM_PATH( It->path().filename() );
        if (Name.GetFileName(false) == rkName) return Name.GetFileExtension();
    }

    return "";
}

}
