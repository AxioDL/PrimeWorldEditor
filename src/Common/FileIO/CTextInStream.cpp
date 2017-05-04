#include "CTextInStream.h"
#include <stdarg.h>
#include <stdio.h>

CTextInStream::CTextInStream()
    : mpFStream(nullptr)
{
}

CTextInStream::CTextInStream(const TString& rkFile)
    : mpFStream(nullptr)
{
    Open(rkFile);
}

CTextInStream::CTextInStream(const CTextInStream& rkSrc)
    : mpFStream(nullptr)
{
    Open(rkSrc.mFileName);

    if (rkSrc.IsValid())
        Seek(rkSrc.Tell(), SEEK_SET);
}

CTextInStream::~CTextInStream()
{
    if (IsValid())
        Close();
}

void CTextInStream::Open(const TString& rkFile)
{
    if (IsValid())
        Close();

    TWideString WideFile = rkFile.ToUTF16();
    _wfopen_s(&mpFStream, *WideFile, L"r");
    mFileName = rkFile;

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
        fclose(mpFStream);
    mpFStream = nullptr;
}

void CTextInStream::Scan(const char *pkFormat, ... )
{
    if (!IsValid()) return;

    va_list Args;
    va_start(Args, pkFormat);
    vfscanf(mpFStream, pkFormat, Args);
}

char CTextInStream::GetChar()
{
    if (!IsValid()) return 0;
    return (char) fgetc(mpFStream);
}

TString CTextInStream::GetString()
{
    if (!IsValid()) return "";

    char Buf[0x1000];
    fgets(Buf, 0x1000, mpFStream);
    return std::string(Buf);
}

u32 CTextInStream::Seek(s32 Offset, u32 Origin)
{
    if (!IsValid()) return 1;
    return fseek(mpFStream, Offset, Origin);
}

u32 CTextInStream::Tell() const
{
    if (!IsValid()) return 0;
    return ftell(mpFStream);
}

bool CTextInStream::EoF() const
{
    return (Tell() == mFileSize);
}

bool CTextInStream::IsValid() const
{
    return (mpFStream != 0);
}

u32 CTextInStream::Size() const
{
    return mFileSize;
}
