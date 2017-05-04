#ifndef CMEMORYOUTSTREAM_H
#define CMEMORYOUTSTREAM_H

#include "IOutputStream.h"

class CMemoryOutStream : public IOutputStream
{
    char *mpDataStart;
    u32 mDataSize;
    u32 mPos;
    u32 mUsed;

public:
    CMemoryOutStream();
    CMemoryOutStream(void *pData, u32 Size, IOUtil::EEndianness mDataEndianness);
    ~CMemoryOutStream();
    void SetData(void *pData, u32 Size, IOUtil::EEndianness mDataEndianness);

    void WriteBytes(const void *pkSrc, u32 Count);
    bool Seek(s32 Offset, u32 Origin);
    u32 Tell() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
    u32 SpaceUsed() const;
    void SetSize(u32 Size);
    void* Data() const;
    void* DataAtPosition() const;
};

#endif // CMEMORYOUTSTREAM_H
