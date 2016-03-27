#ifndef IINPUTSTREAM_H
#define IINPUTSTREAM_H

#include "IOUtil.h"
#include <string>
#include <vector>

class IInputStream
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
    std::string ReadString(unsigned long Count);
    std::wstring ReadWString();
    std::wstring ReadWString(unsigned long Count);

    char PeekByte();
    short PeekShort();
    long PeekLong();
    long long PeekLongLong();
    float PeekFloat();
    double PeekDouble();

    void SeekToBoundary(unsigned long Boundary);
    void SetEndianness(IOUtil::EEndianness Endianness);
    void SetSourceString(const std::string& rkSource);
    IOUtil::EEndianness GetEndianness() const;
    std::string GetSourceString() const;

    virtual ~IInputStream();
    virtual void ReadBytes(void *pDst, unsigned long Count) = 0;
    virtual bool Seek(long Offset, long Origin) = 0;
    virtual bool Seek64(long long Offset, long Origin);
    virtual long Tell() const = 0;
    virtual long long Tell64() const;
    virtual bool EoF() const = 0;
    virtual bool IsValid() const = 0;
    virtual long Size() const = 0;
};

#endif // IINPUTSTREAM_H
