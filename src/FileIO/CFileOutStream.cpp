#include "CFileOutStream.h"

CFileOutStream::CFileOutStream()
{
    mpFStream = nullptr;
    mSize = 0;
}

CFileOutStream::CFileOutStream(const std::string& rkFile)
{
    mpFStream = nullptr;
    Open(rkFile, IOUtil::eBigEndian);
}

CFileOutStream::CFileOutStream(const std::string& rkFile, IOUtil::EEndianness FileEndianness)
{
    mpFStream = nullptr;
    Open(rkFile, FileEndianness);
}

CFileOutStream::CFileOutStream(const CFileOutStream& rkSrc)
{
    mpFStream = nullptr;
    Open(rkSrc.mName, rkSrc.mDataEndianness);

    if (rkSrc.IsValid())
        Seek(rkSrc.Tell(), SEEK_SET);
}

CFileOutStream::~CFileOutStream()
{
    if (IsValid())
        Close();
}

void CFileOutStream::Open(const std::string& rkFile, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mpFStream, rkFile.c_str(), "wb");
    mName = rkFile;
    mDataEndianness = FileEndianness;
    mSize = 0;
}

void CFileOutStream::Update(const std::string& rkFile, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mpFStream, rkFile.c_str(), "rb+");
    mName = rkFile;
    mDataEndianness = FileEndianness;
    Seek(0x0, SEEK_END);
    mSize = Tell();
    Seek(0x0, SEEK_SET);
}

void CFileOutStream::Close()
{
    if (IsValid())
        fclose(mpFStream);
    mpFStream = nullptr;
    mSize = 0;
}

void CFileOutStream::WriteBytes(void *pSrc, unsigned long Count)
{
    if (!IsValid()) return;
    fwrite(pSrc, 1, Count, mpFStream);
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

bool CFileOutStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mpFStream, Offset, Origin) != 0);
}

bool CFileOutStream::Seek64(long long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (_fseeki64(mpFStream, Offset, Origin) != 0);
}

long CFileOutStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mpFStream);
}

long long CFileOutStream::Tell64() const
{
    if (!IsValid()) return 0;
    return _ftelli64(mpFStream);
}

bool CFileOutStream::EoF() const
{
    return (Tell() == Size());
}

bool CFileOutStream::IsValid() const
{
    return (mpFStream != 0);
}

long CFileOutStream::Size() const
{
    if (!IsValid()) return 0;
    return mSize;
}

std::string CFileOutStream::FileName() const
{
    return mName;
}
