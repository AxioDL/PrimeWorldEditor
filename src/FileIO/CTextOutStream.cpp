#include "CTextOutStream.h"
#include <stdarg.h>

CTextOutStream::CTextOutStream()
{
    mFStream = nullptr;
    mSize = 0;
}

CTextOutStream::CTextOutStream(std::string File)
{
    mFStream = nullptr;
    Open(File.c_str());
}

CTextOutStream::CTextOutStream(CTextOutStream& src)
{
    mFStream = nullptr;
    Open(src.mFileName);

    if (src.IsValid())
        Seek(src.Tell(), SEEK_SET);
}

CTextOutStream::~CTextOutStream()
{
    if (IsValid())
        Close();
}

void CTextOutStream::Open(std::string File)
{
    fopen_s(&mFStream, File.c_str(), "w");
    mFileName = File;
    mSize = 0;
}

void CTextOutStream::Close()
{
    if (IsValid())
        fclose(mFStream);
    mFStream = nullptr;
    mSize = 0;
}

void CTextOutStream::Print(const char *Format, ... )
{
    if (!IsValid()) return;

    va_list Args;
    va_start(Args, Format);
    vfprintf(mFStream, Format, Args);
}

void CTextOutStream::WriteChar(char c)
{
    if (!IsValid()) return;
    fputc(c, mFStream);
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

void CTextOutStream::WriteString(std::string Str)
{
    if (!IsValid()) return;
    fputs(Str.c_str(), mFStream);
    if ((unsigned long) Tell() > mSize) mSize = Tell();
}

bool CTextOutStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return false;
    return (fseek(mFStream, Offset, Origin) != 0);
}

long CTextOutStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mFStream);
}

bool CTextOutStream::EoF() const
{
    return (Tell() == mSize);
}

bool CTextOutStream::IsValid() const
{
    return (mFStream != 0);
}

long CTextOutStream::Size() const
{
    return mSize;
}
