#ifndef CVECTOROUTSTREAM_H
#define CVECTOROUTSTREAM_H

#include "IOutputStream.h"
#include <vector>

class CVectorOutStream : public IOutputStream
{
    static const u32 skAllocSize = 1; // must be 1 or a power of 2

    std::vector<char> *mpVector;
    bool mOwnsVector;
    u32 mPos;

public:
    CVectorOutStream();
    CVectorOutStream(IOUtil::EEndianness DataEndianness);
    CVectorOutStream(u32 InitialSize, IOUtil::EEndianness DataEndianness);
    CVectorOutStream(std::vector<char> *pVector, IOUtil::EEndianness DataEndianness);
    ~CVectorOutStream();

    void WriteBytes(const void *pkSrc, u32 Count);
    bool Seek(s32 Offset, u32 Origin);
    u32 Tell() const;
    bool EoF() const;
    bool IsValid() const;
    u32 Size() const;
    void SetVector(std::vector<char> *pVector);
    void *Data();
    void *DataAtPosition();
    void Clear();
};

#endif // CVECTOROUTSTREAM_H
