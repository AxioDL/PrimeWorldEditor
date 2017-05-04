#ifndef CTEXTINSTREAM_H
#define CTEXTINSTREAM_H

#include <cstdio>
#include "Common/TString.h"

class CTextInStream
{
    FILE *mpFStream;
    TString mFileName;
    long mFileSize;

public:
    CTextInStream();
    CTextInStream(const TString& rkFile);
    CTextInStream(const CTextInStream& rkSrc);
    ~CTextInStream();
    void Open(const TString& rkFile);
    void Close();

    void Scan(const char *pkFormat, ... );
    char GetChar();
    TString GetString();

    u32 Seek(s32 Offset, u32 Origin);
    u32 Tell() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
};

#endif // CTEXTINSTREAM_H
