#ifndef CTEXTINSTREAM_H
#define CTEXTINSTREAM_H

#include <cstdio>
#include <string>

class CTextInStream
{
    FILE *mpFStream;
    std::string mFileName;
    long mFileSize;

public:
    CTextInStream();
    CTextInStream(const std::string& rkFile);
    CTextInStream(const CTextInStream& rkSrc);
    ~CTextInStream();
    void Open(const std::string& rkFile);
    void Close();

    void Scan(const char *pkFormat, ... );
    char GetChar();
    std::string GetString();

    long Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
};

#endif // CTEXTINSTREAM_H
