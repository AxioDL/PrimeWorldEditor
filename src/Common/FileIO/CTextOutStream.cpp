#include "CTextOutStream.h"
#include <stdarg.h>

CTextOutStream::CTextOutStream()
    : mpFStream(nullptr)
    , mSize(0)
{
}

CTextOutStream::CTextOutStream(const TString& rkFile)
    : mpFStream(nullptr)
{
    Open(rkFile);
}

CTextOutStream::CTextOutStream(const CTextOutStream& rkSrc)
    : mpFStream(nullptr)
{
    Open(rkSrc.mFileName);

    if (rkSrc.IsValid())
        Seek(rkSrc.Tell(), SEEK_SET);
}

CTextOutStream::~CTextOutStream()
{
    if (IsValid())
        Close();
}

void CTextOutStream::Open(const TString& rkFile)
{
    TWideString WideFile = rkFile.ToUTF16();
    _wfopen_s(&mpFStream, *WideFile, L"w");
    mFileName = rkFile;
    mSize = 0;
}

void CTextOutStream::Close()
{
    if (IsValid())
        fclose(mpFStream);
    mpFStream = nullptr;
    mSize = 0;
}

void CTextOutStream::Print(const char *pkFormat, ... )
{
    if (!IsValid()) return;

    va_list Args;
    va_start(Args, pkFormat);
    vfprintf(mpFStream, pkFormat, Args);
}

void CTextOutStream::WriteChar(char Chr)
{
    if (!IsValid()) return;
    fputc(Chr, mpFStream);
    if (Tell() > mSize) mSize = Tell();
}

void CTextOutStream::WriteString(const TString& rkStr)
{
    if (!IsValid()) return;
    fputs(*rkStr, mpFStream);
    if (Tell() > mSize) mSize = Tell();
}

bool CTextOutStream::Seek(s32 Offset, u32 Origin)
{
    if (!IsValid()) return false;
    return (fseek(mpFStream, Offset, Origin) != 0);
}

u32 CTextOutStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mpFStream);
}

bool CTextOutStream::EoF() const
{
    return (Tell() == mSize);
}

bool CTextOutStream::IsValid() const
{
    return (mpFStream != 0);
}

u32 CTextOutStream::Size() const
{
    return mSize;
}
