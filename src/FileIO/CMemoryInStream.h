#ifndef CMEMORYINSTREAM_H
#define CMEMORYINSTREAM_H

#include "CInputStream.h"
#include "IOUtil.h"

class CMemoryInStream : public CInputStream
{
    char *mDataStart;
    long mDataSize;
    long mPos;

public:
    CMemoryInStream();
    CMemoryInStream(void *Data, unsigned long Size, IOUtil::EEndianness dataEndianness);
    ~CMemoryInStream();
    void SetData(void *Data, unsigned long Size, IOUtil::EEndianness dataEndianness);

    void ReadBytes(void *dst, unsigned long Count);
    bool Seek(long offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    void SetSize(unsigned long Size);
    void* Data() const;
    void* DataAtPosition() const;
};

#endif // CMEMORYINSTREAM_H
