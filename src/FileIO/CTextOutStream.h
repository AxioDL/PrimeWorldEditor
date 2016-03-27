#ifndef CTEXTOUTSTREAM_H
#define CTEXTOUTSTREAM_H

#include <string>

class CTextOutStream
{
    FILE *mpFStream;
    std::string mFileName;
    unsigned long mSize;

public:
    CTextOutStream();
    CTextOutStream(const std::string& rkFile);
    CTextOutStream(const CTextOutStream& rkSrc);
    ~CTextOutStream();
    void Open(const std::string& rkFile);
    void Close();

    void Print(const char *pkFormat, ... );
    void WriteChar(char Chr);
    void WriteString(const std::string& rkStr);

    bool Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
};

#endif // CTEXTOUTSTREAM_H
