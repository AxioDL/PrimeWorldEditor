#ifndef CFILEINSTREAM_H
#define CFILEINSTREAM_H

#include "CInputStream.h"
#include "IOUtil.h"

class CFileInStream : public CInputStream
{
private:
    FILE *mFStream;
    std::string mName;
    long mFileSize;

public:
    CFileInStream();
    CFileInStream(std::string file);
    CFileInStream(std::string file, IOUtil::EEndianness FileEndianness);
    CFileInStream(CFileInStream& src);
    ~CFileInStream();
    void Open(std::string file, IOUtil::EEndianness FileEndianness);
    void Close();

    void ReadBytes(void *dst, unsigned long count);
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
