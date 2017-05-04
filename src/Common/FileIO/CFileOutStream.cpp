#include "CFileOutStream.h"

CFileOutStream::CFileOutStream()
    : mpFStream(nullptr)
    , mSize(0)
{
}

CFileOutStream::CFileOutStream(const TString& rkFile)
    : mpFStream(nullptr)
{
    Open(rkFile, IOUtil::eBigEndian);
}

CFileOutStream::CFileOutStream(const TString& rkFile, IOUtil::EEndianness FileEndianness)
    : mpFStream(nullptr)
{
    Open(rkFile, FileEndianness);
}

CFileOutStream::CFileOutStream(const CFileOutStream& rkSrc)
    : mpFStream(nullptr)
{
    Open(rkSrc.mName, rkSrc.mDataEndianness);

    if (rkSrc.IsValid())
        Seek(rkSrc.Tell(), SEEK_SET);
}

CFileOutStream::~CFileOutStream()
{
    if (IsValid())
        Close();
}

void CFileOutStream::Open(const TString& rkFile, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    TWideString WideFile = rkFile.ToUTF16();
    _wfopen_s(&mpFStream, *WideFile, L"wb");
    mName = rkFile;
    mDataEndianness = FileEndianness;
    mSize = 0;
}

void CFileOutStream::Update(const TString& rkFile, IOUtil::EEndianness FileEndianness)
{
    if (IsValid())
        Close();

    TWideString WideFile = rkFile.ToUTF16();
    _wfopen_s(&mpFStream, *WideFile, L"rb+");
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

void CFileOutStream::WriteBytes(const void *pkSrc, u32 Count)
{
    if (!IsValid()) return;
    fwrite(pkSrc, 1, Count, mpFStream);
    if (Tell() > mSize) mSize = Tell();
}

bool CFileOutStream::Seek(s32 Offset, u32 Origin)
{
    if (!IsValid()) return false;
    return (fseek(mpFStream, Offset, Origin) != 0);
}

bool CFileOutStream::Seek64(s64 Offset, u32 Origin)
{
    if (!IsValid()) return false;
    return (_fseeki64(mpFStream, Offset, Origin) != 0);
}

u32 CFileOutStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mpFStream);
}

u64 CFileOutStream::Tell64() const
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

u32 CFileOutStream::Size() const
{
    if (!IsValid()) return 0;
    return mSize;
}

TString CFileOutStream::FileName() const
{
    return mName;
}
