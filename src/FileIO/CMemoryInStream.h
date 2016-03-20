#ifndef CMEMORYINSTREAM_H
#define CMEMORYINSTREAM_H

#include "IInputStream.h"
#include "IOUtil.h"

class CMemoryInStream : public IInputStream
{
    const char *mpDataStart;
    long mDataSize;
    long mPos;

public:
    CMemoryInStream();
    CMemoryInStream(const void *pData, unsigned long Size, IOUtil::EEndianness dataEndianness);
    ~CMemoryInStream();
    void SetData(const void *pData, unsigned long Size, IOUtil::EEndianness dataEndianness);

    void ReadBytes(void *pDst, unsigned long Count);
    bool Seek(long offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    void SetSize(unsigned long Size);
    const void* Data() const;
    const void* DataAtPosition() const;
};

#endif // CMEMORYINSTREAM_H
