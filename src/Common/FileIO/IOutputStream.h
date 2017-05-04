#ifndef IOUTPUTSTREAM_H
#define IOUTPUTSTREAM_H

#include "IOUtil.h"
#include "Common/TString.h"

class IOutputStream
{
protected:
    IOUtil::EEndianness mDataEndianness;
    TString mDataDest;

public:
    void WriteBool(bool Val);
    void WriteByte(char Val);
    void WriteShort(short Val);
    void WriteLong(long Val);
    void WriteLongLong(long long Val);
    void WriteFloat(float Val);
    void WriteDouble(double Val);
    void WriteFourCC(long Val);
    void WriteString(const TString& rkVal);
    void WriteString(const TString& rkVal, u32 Count, bool Terminate = false);
    void WriteSizedString(const TString& rkVal);
    void WriteWideString(const TWideString& rkVal);
    void WriteWideString(const TWideString& rkVal, u32 Count, bool Terminate = false);
    void WriteSizedWideString(const TWideString& rkVal);

    bool GoTo(u32 Address);
    bool Skip(s32 SkipAmount);

    void WriteToBoundary(u32 Boundary, u8 Fill);
    void SetEndianness(IOUtil::EEndianness Endianness);
    void SetDestString(const TString& rkDest);
    IOUtil::EEndianness GetEndianness() const;
    TString GetDestString() const;

    virtual ~IOutputStream();
    virtual void WriteBytes(const void *pkSrc, u32 Count) = 0;
    virtual bool Seek(s32 Offset, u32 Origin) = 0;
    virtual bool Seek64(s64 Offset, u32 Origin);
    virtual u32 Tell() const = 0;
    virtual u64 Tell64() const;
    virtual bool EoF() const = 0;
    virtual bool IsValid() const = 0;
    virtual u32 Size() const = 0;
};
#endif // COUTPUTSTREAM_H
