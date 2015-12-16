#ifndef CFILEOUTSTREAM_H
#define CFILEOUTSTREAM_H

#include "IOutputStream.h"
#include "IOUtil.h"

class CFileOutStream : public IOutputStream
{
private:
    FILE *mpFStream;
    std::string mName;
    unsigned long mSize;

public:
    CFileOutStream();
    CFileOutStream(const std::string& rkFile);
    CFileOutStream(const std::string& rkFile, IOUtil::EEndianness FileEndianness);
    CFileOutStream(const CFileOutStream& rkSrc);
    ~CFileOutStream();
    void Open(const std::string& rkFile, IOUtil::EEndianness);
    void Update(const std::string& rkFile, IOUtil::EEndianness FileEndianness);
    void Close();

    void WriteBytes(void *pSrc, unsigned long count);
    bool Seek(long offset, long origin);
    bool Seek64(long long offset, long origin);
    long Tell() const;
    long long Tell64() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    std::string FileName() const;
};

#endif // CFILEOUTSTREAM_H
