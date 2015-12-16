#ifndef CINPUTSTREAM_H
#define CINPUTSTREAM_H

#include "IOUtil.h"
#include <string>
#include <vector>

class CInputStream
{
protected:
    IOUtil::EEndianness mDataEndianness;
    std::string mDataSource;
    
public:
    char ReadByte();
    short ReadShort();
    long ReadLong();
    long long ReadLongLong();
    float ReadFloat();
    double ReadDouble();
    std::string ReadString();
    std::string ReadString(unsigned long count);
    std::wstring ReadWString();
    std::wstring ReadWString(unsigned long count);

    char PeekByte();
    short PeekShort();
    long PeekLong();
    long long PeekLongLong();
    float PeekFloat();
    double PeekDouble();

    void SeekToBoundary(unsigned long boundary);
    void SetEndianness(IOUtil::EEndianness endianness);
    void SetSourceString(const std::string& source);
    IOUtil::EEndianness GetEndianness() const;
    std::string GetSourceString() const;

    virtual ~CInputStream();
    virtual void ReadBytes(void *dst, unsigned long count) = 0;
    virtual bool Seek(long offset, long origin) = 0;
    virtual bool Seek64(long long offset, long origin);
    virtual long Tell() const = 0;
    virtual long long Tell64() const;
    virtual bool EoF() const = 0;
    virtual bool IsValid() const = 0;
    virtual long Size() const = 0;
};

#endif // CINPUTSTREAM_H
