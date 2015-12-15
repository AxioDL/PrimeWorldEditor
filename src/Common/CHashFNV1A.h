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

    static const u64 skFNVOffsetBasis32;
    static const u64 skFNVOffsetBasis64;
    static const u64 skFNVPrime32;
    static const u64 skFNVPrime64;

public:
    CHashFNV1A();
    void Init32();
    void Init64();
    void HashData(const void *pData, u32 Size);
    u32 GetHash32();
    u64 GetHash64();

    // Convenience functions
    void HashByte(const u8& v);
    void HashShort(const u16& v);
    void HashLong(const u32& v);
    void HashFloat(const float& v);
    void HashString(const TString& v);
};

#endif // CHASHFNV1A_H
