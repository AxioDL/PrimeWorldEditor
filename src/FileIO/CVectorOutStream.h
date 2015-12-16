#ifndef CVECTOROUTSTREAM_H
#define CVECTOROUTSTREAM_H

#include "COutputStream.h"
#include <vector>

class CVectorOutStream : public COutputStream
{
    std::vector<char> mVector;
    long mPos;
    long mUsed;

public:
    CVectorOutStream();
    CVectorOutStream(IOUtil::EEndianness DataEndianness);
    CVectorOutStream(unsigned long InitialSize, IOUtil::EEndianness DataEndianness);
    ~CVectorOutStream();

    void WriteBytes(void *src, unsigned long Count);
    bool Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    long SizeRemaining() const;
    void *Data();
    void *DataAtPosition();
    void Expand(unsigned long Amount);
    void Contract();
    void Reset();
    void Clear();
};

#endif // CVECTOROUTSTREAM_H
