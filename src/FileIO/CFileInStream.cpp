#include "CFileInStream.h"

CFileInStream::CFileInStream()
{
    mpFStream = nullptr;
}

CFileInStream::CFileInStream(const std::string& rkFile)
{
    mpFStream = nullptr;
    Open(rkFile, IOUtil::eBigEndian);
}

CFileInStream::CFileInStream(const std::string& rkFile, IOUtil::EEndianness FileEndianness)
{
    mpFStream = nullptr;
    Open(rkFile, FileEndianness);
}

CFileInStream::CFileInStream(const CFileInStream& rkSrc)
{
    mpFStream = nullptr;
    Open(rkSrc.mName, rkSrc.mDataEndianness);

    if (rkSrc.IsValid())
        Seek(rkSrc.Tell(), SEEK_SET);
}

CFileInStream::~CFileInStream()
{
    if (IsValid())
        Close();
}

void CFileInStream::Open(const std::string& rkFile, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mpFStream, rkFile.c_str(), "rb");
    mName = rkFile;
    mDataEndianness = FileEndianness;

    if (IsValid())
    {
        Seek(0x0, SEEK_END);
        mFileSize = Tell();
        Seek(0x0, SEEK_SET);
    }
    else
        mFileSize = 0;

    size_t EndPath = rkFile.find_last_of("\\/");
    SetSourceString(rkFile.substr(EndPath + 1, rkFile.length() - EndPath));
}

void CFileInStream::Close()
{
    if (IsValid())
        fclose(mpFStream);
    mpFStream = nullptr;
}

void CFileInStream::ReadBytes(void *pDst, unsigned long Count)
{
    if (!IsValid()) return;
    fread(pDst, 1, Count, mpFStream);
}

bool CFileInStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mpFStream, Offset, Origin) != 0);
}

bool CFileInStream::Seek64(long long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (_fseeki64(mpFStream, Offset, Origin) != 0);
}

long CFileInStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mpFStream);
}

long long CFileInStream::Tell64() const
{
    if (!IsValid()) return 0;
    return _ftelli64(mpFStream);
}

bool CFileInStream::EoF() const
{
    return (Tell() >= mFileSize);
}

bool CFileInStream::IsValid() const
{
    return (mpFStream != 0);
}

long CFileInStream::Size() const
{
    return mFileSize;
}

std::string CFileInStream::FileName() const
{
    return mName;
}
