#ifndef CFILEOUTSTREAM_H
#define CFILEOUTSTREAM_H

#include "IOutputStream.h"

class CFileOutStream : public IOutputStream
{
private:
    FILE *mpFStream;
    TString mName;
    u32 mSize;

public:
    CFileOutStream();
    CFileOutStream(const TString& rkFile);
    CFileOutStream(const TString& rkFile, IOUtil::EEndianness FileEndianness);
    CFileOutStream(const CFileOutStream& rkSrc);
    ~CFileOutStream();
    void Open(const TString& rkFile, IOUtil::EEndianness);
    void Update(const TString& rkFile, IOUtil::EEndianness FileEndianness);
    void Close();

    void WriteBytes(const void *pkSrc, u32 Count);
    bool Seek(s32 Offset, u32 Origin);
    bool Seek64(s64 Offset, u32 Origin);
    u32 Tell() const;
    u64 Tell64() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
    TString FileName() const;
};

#endif // CFILEOUTSTREAM_H
