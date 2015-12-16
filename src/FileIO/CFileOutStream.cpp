#include "CFileOutStream.h"

CFileOutStream::CFileOutStream()
{
    mFStream = nullptr;
    mSize = 0;
}

CFileOutStream::CFileOutStream(std::string File)
{
    mFStream = nullptr;
    Open(File, IOUtil::BigEndian);
}

CFileOutStream::CFileOutStream(std::string File, IOUtil::EEndianness FileEndianness)
{
    mFStream = nullptr;
    Open(File, FileEndianness);
}

CFileOutStream::CFileOutStream(CFileOutStream& src)
{
    mFStream = nullptr;
    Open(src.mName, src.mDataEndianness);

    if (src.IsValid())
        Seek(src.Tell(), SEEK_SET);
}

CFileOutStream::~CFileOutStream()
{
    if (IsValid())
        Close();
}

void CFileOutStream::Open(std::string File, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mFStream, File.c_str(), "wb");
    mName = File;
    mDataEndianness = FileEndianness;
    mSize = 0;
}

void CFileOutStream::Update(std::string File, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mFStream, File.c_str(), "rb+");
    mName = File;
    mDataEndianness = FileEndianness;
    Seek(0x0, SEEK_END);
    mSize = Tell();
    Seek(0x0, SEEK_SET);
}

void CFileOutStream::Close()
{
    if (IsValid())
        fclose(mFStream);
    mFStream = nullptr;
    mSize = 0;
}

void CFileOutStream::WriteBytes(void *src, unsigned long Count)
{
    if (!IsValid()) return;
    fwrite(src, 1, Count, mFStream);
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

bool CFileOutStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mFStream, Offset, Origin) != 0);
}

bool CFileOutStream::Seek64(long long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (_fseeki64(mFStream, Offset, Origin) != 0);
}

long CFileOutStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mFStream);
}

long long CFileOutStream::Tell64() const
{
    if (!IsValid()) return 0;
    return _ftelli64(mFStream);
}

bool CFileOutStream::EoF() const
{
    return (Tell() == Size());
}

bool CFileOutStream::IsValid() const
{
    return (mFStream != 0);
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
