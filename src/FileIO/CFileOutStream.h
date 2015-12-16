#ifndef CFILEOUTSTREAM_H
#define CFILEOUTSTREAM_H

#include "COutputStream.h"
#include "IOUtil.h"

class CFileOutStream : public COutputStream
{
private:
    FILE *mFStream;
    std::string mName;
    unsigned long mSize;

public:
    CFileOutStream();
    CFileOutStream(std::string file);
    CFileOutStream(std::string file, IOUtil::EEndianness FileEndianness);
    CFileOutStream(CFileOutStream& src);
    ~CFileOutStream();
    void Open(std::string file, IOUtil::EEndianness);
    void Update(std::string file, IOUtil::EEndianness FileEndianness);
    void Close();

    void WriteBytes(void *src, unsigned long count);
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
