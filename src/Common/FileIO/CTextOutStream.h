#ifndef CTEXTOUTSTREAM_H
#define CTEXTOUTSTREAM_H

#include "Common/TString.h"

class CTextOutStream
{
    FILE *mpFStream;
    TString mFileName;
    unsigned long mSize;

public:
    CTextOutStream();
    CTextOutStream(const TString& rkFile);
    CTextOutStream(const CTextOutStream& rkSrc);
    ~CTextOutStream();
    void Open(const TString& rkFile);
    void Close();

    void Print(const char *pkFormat, ... );
    void WriteChar(char Chr);
    void WriteString(const TString& rkStr);

    bool Seek(s32 Offset, u32 Origin);
    u32 Tell() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
};

#endif // CTEXTOUTSTREAM_H
