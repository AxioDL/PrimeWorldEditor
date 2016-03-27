#ifndef CMEMORYOUTSTREAM_H
#define CMEMORYOUTSTREAM_H

#include "IOutputStream.h"

class CMemoryOutStream : public IOutputStream
{
    char *mpDataStart;
    long mDataSize;
    long mPos;
    long mUsed;

public:
    CMemoryOutStream();
    CMemoryOutStream(void *pData, unsigned long Size, IOUtil::EEndianness mDataEndianness);
    ~CMemoryOutStream();
    void SetData(void *pData, unsigned long Size, IOUtil::EEndianness mDataEndianness);

    void WriteBytes(void *pSrc, unsigned long Count);
    bool Seek(long Offset, long Origin);
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
