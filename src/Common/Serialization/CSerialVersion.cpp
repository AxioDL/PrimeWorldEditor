#include "CSerialVersion.h"
#include "Common/CFourCC.h"

CSerialVersion::CSerialVersion()
{
}

CSerialVersion::CSerialVersion(u16 ArchiveVer, u16 FileVer, EGame Game)
    : mArchiveVersion(ArchiveVer)
    , mFileVersion(FileVer)
    , mGame(Game)
{
}

CSerialVersion::CSerialVersion(IInputStream& rInput)
{
    Read(rInput);
}

void CSerialVersion::Read(IInputStream& rInput)
{
    mArchiveVersion = rInput.ReadShort();
    mFileVersion = rInput.ReadShort();
    CFourCC GameID(rInput);
    mGame = GetGameForID(GameID);
}

void CSerialVersion::Write(IOutputStream& rOutput)
{
    rOutput.WriteShort(mArchiveVersion);
    rOutput.WriteShort(mFileVersion);
    CFourCC GameID = GetGameID(mGame);
    GameID.Write(rOutput);
}
