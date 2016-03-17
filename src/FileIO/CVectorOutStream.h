#ifndef CVECTOROUTSTREAM_H
#define CVECTOROUTSTREAM_H

#include "IOutputStream.h"
#include <vector>

class CVectorOutStream : public IOutputStream
{
    std::vector<char> *mpVector;
    bool mOwnsVector;
    long mPos;
    long mUsed;

public:
    CVectorOutStream();
    CVectorOutStream(IOUtil::EEndianness DataEndianness);
    CVectorOutStream(unsigned long InitialSize, IOUtil::EEndianness DataEndianness);
    CVectorOutStream(std::vector<char> *pVector, IOUtil::EEndianness DataEndianness);
    ~CVectorOutStream();

    void WriteBytes(void *pSrc, unsigned long Count);
    bool Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    long SizeRemaining() const;
    void SetVector(std::vector<char> *pVector);
    void *Data();
    void *DataAtPosition();
    void Expand(unsigned long Amount);
    void Shrink();
    void Reset();
    void Clear();
};

#endif // CVECTOROUTSTREAM_H
