#ifndef CMEMORYOUTSTREAM_H
#define CMEMORYOUTSTREAM_H

#include "COutputStream.h"

class CMemoryOutStream : public COutputStream
{
    char *mDataStart;
    long mDataSize;
    long mPos;
    long mUsed;

public:
    CMemoryOutStream();
    CMemoryOutStream(void *Data, unsigned long Size, IOUtil::EEndianness mDataEndianness);
    ~CMemoryOutStream();
    void SetData(void *Data, unsigned long Size, IOUtil::EEndianness mDataEndianness);

    void WriteBytes(void *src, unsigned long count);
    bool Seek(long offset, long origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    long SpaceUsed() const;
    void SetSize(unsigned long Size);
    void* Data() const;
    void* DataAtPosition() const;
};

#endif // CMEMORYOUTSTREAM_H
