#ifndef CFNV1A_H
#define CFNV1A_H

#include "Common/types.h"
#include "Common/TString.h"

class CFNV1A
{
public:
    enum EHashLength {
        e32Bit, e64Bit
    };

private:
    u64 mHash;
    EHashLength mHashLength;

    static const u64 skFNVOffsetBasis32 = 0x811C9DC5;
    static const u64 skFNVOffsetBasis64 = 0xCBF29CE484222325;
    static const u64 skFNVPrime32 = 0x1000193;
    static const u64 skFNVPrime64 = 0x100000001B3;

public:
    CFNV1A(EHashLength Length)
    {
        if (Length == e32Bit)
            Init32();
        else
            Init64();
    }

    void Init32()
    {
        mHashLength = e32Bit;
        mHash = skFNVOffsetBasis32;
    }

    void Init64()
    {
        mHashLength = e64Bit;
        mHash = skFNVOffsetBasis64;
    }

    void HashData(const void *pkData, u32 Size)
    {
        const char *pkCharData = (const char*) pkData;
        u64 FNVPrime = (mHashLength == e32Bit) ? skFNVPrime32 : skFNVPrime64;

        for (u32 iByte = 0; iByte < Size; iByte++)
        {
            mHash ^= *pkCharData;
            mHash *= FNVPrime;
            pkCharData++;
        }
    }

    inline u32 GetHash32()  { return (u32) mHash; }
    inline u64 GetHash64()  { return mHash; }

    // Convenience functions
    inline void HashByte(const u8& rkVal)           { HashData(&rkVal, 1); }
    inline void HashShort(const u16& rkVal)         { HashData(&rkVal, 2); }
    inline void HashLong(const u32& rkVal)          { HashData(&rkVal, 4); }
    inline void HashFloat(const float& rkVal)       { HashData(&rkVal, 4); }
    inline void HashString(const TString& rkVal)    { HashData(rkVal.Data(), rkVal.Size()); }
};

#endif // CFNV1A_H
