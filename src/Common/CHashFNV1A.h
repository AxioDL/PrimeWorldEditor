#ifndef CHASHFNV1A_H
#define CHASHFNV1A_H

#include "types.h"
#include "TString.h"

class CHashFNV1A
{
    u64 mHash;

    enum EHashLength {
        e32Bit, e64Bit
    } mHashLength;

    static const u64 skFNVOffsetBasis32 = 0x811C9DC5;
    static const u64 skFNVOffsetBasis64 = 0xCBF29CE484222325;
    static const u64 skFNVPrime32 = 0x1000193;
    static const u64 skFNVPrime64 = 0x100000001B3;

public:
    CHashFNV1A()
    {
        Init32();
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

#endif // CHASHFNV1A_H
