#ifndef COUTPUTSTREAM_H
#define COUTPUTSTREAM_H

#include "IOUtil.h"
#include <string>

class COutputStream
{
protected:
    IOUtil::EEndianness mDataEndianness;
    std::string mDataDest;

public:
    void WriteByte(char Val);
    void WriteShort(short Val);
    void WriteLong(long Val);
    void WriteLongLong(long long Val);
    void WriteFloat(float Val);
    void WriteDouble(double Val);
    void WriteString(std::string Val);
    void WriteString(std::string Val, unsigned long Count, bool Terminate = false);
    void WriteWString(std::wstring Val);
    void WriteWString(std::wstring Val, unsigned long Count, bool Terminate = false);

    void WriteToBoundary(unsigned long Boundary, char Fill);
    void SetEndianness(IOUtil::EEndianness Endianness);
    void SetDestString(const std::string& Dest);
    IOUtil::EEndianness GetEndianness() const;
    std::string GetDestString() const;

    virtual ~COutputStream();
    virtual void WriteBytes(void *src, unsigned long Count) = 0;
    virtual bool Seek(long Offset, long Origin) = 0;
    virtual bool Seek64(long long Offset, long Origin);
    virtual long Tell() const = 0;
    virtual long long Tell64() const;
    virtual bool EoF() const = 0;
    virtual bool IsValid() const = 0;
    virtual long Size() const = 0;
};
#endif // COUTPUTSTREAM_H
