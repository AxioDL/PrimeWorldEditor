#ifndef CTEXTOUTSTREAM_H
#define CTEXTOUTSTREAM_H

#include <string>

class CTextOutStream
{
    FILE *mFStream;
    std::string mFileName;
    unsigned long mSize;

public:
    CTextOutStream();
    CTextOutStream(std::string File);
    CTextOutStream(CTextOutStream& src);
    ~CTextOutStream();
    void Open(std::string file);
    void Close();

    void Print(const char *Format, ... );
    void WriteChar(char c);
    void WriteString(std::string Str);

    bool Seek(long Offset, long Origin);
    long Tell() const;
    bool EoF() const;
    bool IsValid() const;
    long Size() const;
};

#endif // CTEXTOUTSTREAM_H
