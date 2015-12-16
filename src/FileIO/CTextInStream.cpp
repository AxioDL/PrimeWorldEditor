#include "CTextInStream.h"
#include <stdarg.h>
#include <stdio.h>

CTextInStream::CTextInStream(std::string File)
{
    mFStream = nullptr;
    Open(File);
}

CTextInStream::CTextInStream(CTextInStream& src)
{
    mFStream = nullptr;
    Open(src.mFileName);

    if (src.IsValid())
        Seek(src.Tell(), SEEK_SET);
}

CTextInStream::~CTextInStream()
{
    if (IsValid())
        Close();
}

void CTextInStream::Open(std::string File)
{
    if (IsValid())
        Close();

    fopen_s(&mFStream, File.c_str(), "r");
    mFileName = File;

    if (IsValid())
    {
        Seek(0x0, SEEK_END);
        mFileSize = Tell();
        Seek(0x0, SEEK_SET);
    }
    else
        mFileSize = 0;
}

void CTextInStream::Close()
{
    if (IsValid())
        fclose(mFStream);
    mFStream = nullptr;
}

void CTextInStream::Scan(const char *Format, ... )
{
    if (!IsValid()) return;

    va_list Args;
    va_start(Args, Format);
    vfscanf(mFStream, Format, Args);
}

char CTextInStream::GetChar()
{
    if (!IsValid()) return 0;
    return (char) fgetc(mFStream);
}

std::string CTextInStream::GetString()
{
    if (!IsValid()) return "";

    char Buf[0x1000];
    fgets(Buf, 0x1000, mFStream);
    return std::string(Buf);
}

long CTextInStream::Seek(long Offset, long Origin)
{
    if (!IsValid()) return 1;
    return fseek(mFStream, Offset, Origin);
}

long CTextInStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mFStream);
}

bool CTextInStream::EoF() const
{
    return (Tell() == mFileSize);
}

bool CTextInStream::IsValid() const
{
    return (mFStream != 0);
}

long CTextInStream::Size() const
{
    return mFileSize;
}
