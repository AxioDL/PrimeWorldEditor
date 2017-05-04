#ifndef CMEMORYINSTREAM_H
#define CMEMORYINSTREAM_H

#include "IInputStream.h"

class CMemoryInStream : public IInputStream
{
    const char *mpDataStart;
    u32 mDataSize;
    u32 mPos;

public:
    CMemoryInStream();
    CMemoryInStream(const void *pkData, u32 Size, IOUtil::EEndianness dataEndianness);
    ~CMemoryInStream();
    void SetData(const void *pkData, u32 Size, IOUtil::EEndianness dataEndianness);

    void ReadBytes(void *pDst, u32 Count);
    bool Seek(s32 Offset, u32 Origin);
    u32 Tell() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
    void SetSize(u32 Size);
    const void* Data() const;
    const void* DataAtPosition() const;
};

#endif // CMEMORYINSTREAM_H
