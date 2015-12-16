#include "CFileInStream.h"

CFileInStream::CFileInStream()
{
    mFStream = nullptr;
}

CFileInStream::CFileInStream(std::string File)
{
    mFStream = nullptr;
    Open(File, IOUtil::BigEndian);
}

CFileInStream::CFileInStream(std::string File, IOUtil::EEndianness FileEndianness)
{
    mFStream = nullptr;
    Open(File, FileEndianness);
}

CFileInStream::CFileInStream(CFileInStream& src)
{
    mFStream = nullptr;
    Open(src.mName, src.mDataEndianness);

    if (src.IsValid())
        Seek(src.Tell(), SEEK_SET);
}

CFileInStream::~CFileInStream()
{
    if (IsValid())
        Close();
}

void CFileInStream::Open(std::string File, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    fopen_s(&mFStream, File.c_str(), "rb");
    mName = File;
    mDataEndianness = FileEndianness;

    if (IsValid())
    {
        Seek(0x0, SEEK_END);
        mFileSize = Tell();
        Seek(0x0, SEEK_SET);
    }
    else
        mFileSize = 0;

    size_t EndPath = File.find_last_of("\\/");
    SetSourceString(File.substr(EndPath + 1, File.length() - EndPath));
}

void CFileInStream::Close()
{
    if (IsValid())
        fclose(mFStream);
    mFStream = nullptr;
}

void CFileInStream::ReadBytes(void *dst, unsigned long Count)
{
    if (!IsValid()) return;
    fread(dst, 1, Count, mFStream);
}

bool CFileInStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mFStream, Offset, Origin) != 0);
}

bool CFileInStream::Seek64(long long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (_fseeki64(mFStream, Offset, Origin) != 0);
}

long CFileInStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mFStream);
}

long long CFileInStream::Tell64() const
{
    if (!IsValid()) return 0;
    return _ftelli64(mFStream);
}

bool CFileInStream::EoF() const
{
    return (Tell() >= mFileSize);
}

bool CFileInStream::IsValid() const
{
    return (mFStream != 0);
}

long CFileInStream::Size() const
{
    return mFileSize;
}

std::string CFileInStream::FileName() const
{
    return mName;
}
