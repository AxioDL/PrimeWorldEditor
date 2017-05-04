#ifndef CFILEINSTREAM_H
#define CFILEINSTREAM_H

#include "IInputStream.h"
#include "IOUtil.h"

class CFileInStream : public IInputStream
{
private:
    FILE *mpFStream;
    TString mName;
    u32 mFileSize;

public:
    CFileInStream();
    CFileInStream(const TString& rkFile);
    CFileInStream(const TString& rkFile, IOUtil::EEndianness FileEndianness);
    CFileInStream(const CFileInStream& rkSrc);
    ~CFileInStream();
    void Open(const TString& rkFile, IOUtil::EEndianness FileEndianness);
    void Close();

    void ReadBytes(void *pDst, u32 Count);
    bool Seek(s32 Offset, u32 Origin);
    bool Seek64(s64 Offset, u32 Origin);
    u32 Tell() const;
    u64 Tell64() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
    TString FileName() const;
};

#endif // CFILEINSTREAM_H
