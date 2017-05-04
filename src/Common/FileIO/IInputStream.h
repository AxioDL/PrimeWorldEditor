#ifndef IINPUTSTREAM_H
#define IINPUTSTREAM_H

#include "IOUtil.h"
#include "Common/TString.h"
#include "Common/types.h"
#include <vector>

class IInputStream
{
protected:
    IOUtil::EEndianness mDataEndianness;
    TString mDataSource;
    
public:
    bool ReadBool();
    char ReadByte();
    short ReadShort();
    long ReadLong();
    long long ReadLongLong();
    float ReadFloat();
    double ReadDouble();
    long ReadFourCC();
    TString ReadString();
    TString ReadString(u32 Count);
    TString ReadSizedString();
    TWideString ReadWString();
    TWideString ReadWString(u32 Count);
    TWideString ReadSizedWString();

    char PeekByte();
    short PeekShort();
    long PeekLong();
    long long PeekLongLong();
    float PeekFloat();
    double PeekDouble();
    long PeekFourCC();

    bool GoTo(u32 Address);
    bool Skip(s32 SkipAmount);

    void SeekToBoundary(u32 Boundary);
    void SetEndianness(IOUtil::EEndianness Endianness);
    void SetSourceString(const TString& rkSource);
    IOUtil::EEndianness GetEndianness() const;
    TString GetSourceString() const;

    virtual ~IInputStream();
    virtual void ReadBytes(void *pDst, u32 Count) = 0;
    virtual bool Seek(s32 Offset, u32 Origin) = 0;
    virtual bool Seek64(s64 Offset, u32 Origin);
    virtual u32 Tell() const = 0;
    virtual u64 Tell64() const;
    virtual bool EoF() const = 0;
    virtual bool IsValid() const = 0;
    virtual u32 Size() const = 0;
};

#endif // IINPUTSTREAM_H
