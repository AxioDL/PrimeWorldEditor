#include "CTextOutStream.h"
#include <stdarg.h>

CTextOutStream::CTextOutStream()
    : mpFStream(nullptr)
    , mSize(0)
{
}

CTextOutStream::CTextOutStream(const std::string& rkFile)
    : mpFStream(nullptr)
{
    Open(rkFile.c_str());
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

void CTextOutStream::Open(const std::string& rkFile)
{
    fopen_s(&mpFStream, rkFile.c_str(), "w");
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
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

void CTextOutStream::WriteString(const std::string& rkStr)
{
    if (!IsValid()) return;
    fputs(rkStr.c_str(), mpFStream);
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

bool CTextOutStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mpFStream, Offset, Origin) != 0);
}

long CTextOutStream::Tell() const
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

long CTextOutStream::Size() const
{
    return mSize;
}
