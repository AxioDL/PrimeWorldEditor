#ifndef CFILEINSTREAM_H
#define CFILEINSTREAM_H

#include "IInputStream.h"
#include "IOUtil.h"

class CFileInStream : public IInputStream
{
private:
    FILE *mpFStream;
    std::string mName;
    long mFileSize;

public:
    CFileInStream();
    CFileInStream(const std::string& rkFile);
    CFileInStream(const std::string& rkFile, IOUtil::EEndianness FileEndianness);
    CFileInStream(const CFileInStream& rkSrc);
    ~CFileInStream();
    void Open(const std::string& rkFile, IOUtil::EEndianness FileEndianness);
    void Close();

    void ReadBytes(void *pDst, unsigned long count);
    bool Seek(long offset, long origin);
    bool Seek64(long long offset, long origin);
    long Tell() const;
    long long Tell64() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
    std::string FileName() const;
};

#endif // CFILEINSTREAM_H
