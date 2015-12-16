#ifndef CTEXTINSTREAM_H
#define CTEXTINSTREAM_H

#include <cstdio>
#include <string>

class CTextInStream
{
    FILE *mFStream;
    std::string mFileName;
    long mFileSize;

public:
    CTextInStream();
    CTextInStream(std::string File);
    CTextInStream(CTextInStream& src);
    ~CTextInStream();
    void Open(std::string File);
    void Close();

    void Scan(const char *Format, ... );
    char GetChar();
    std::string GetString();

    long Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
};

#endif // CTEXTINSTREAM_H
