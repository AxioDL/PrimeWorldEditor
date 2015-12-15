#include "CHashFNV1A.h"

const u64 CHashFNV1A::skFNVOffsetBasis32 = 0x811C9DC5;
const u64 CHashFNV1A::skFNVOffsetBasis64 = 0xCBF29CE484222325;
const u64 CHashFNV1A::skFNVPrime32 = 0x1000193;
const u64 CHashFNV1A::skFNVPrime64 = 0x100000001B3;

CHashFNV1A::CHashFNV1A()
{
    Init32();
}

void CHashFNV1A::Init32()
{
    mHashLength = e32Bit;
    mHash = skFNVOffsetBasis32;
}

void CHashFNV1A::Init64()
{
    mHashLength = e64Bit;
    mHash = skFNVOffsetBasis64;
}

void CHashFNV1A::HashData(const void *pData, u32 Size)
{
    const char *pCharData = (const char*) pData;
    u64 FNVPrime = (mHashLength == e32Bit) ? skFNVPrime32 : skFNVPrime64;

    for (u32 i = 0; i < Size; i++)
    {
        mHash ^= *pCharData;
        mHash *= FNVPrime;
        pCharData++;
    }
}

u32 CHashFNV1A::GetHash32()
{
    return (u32) mHash;
}

u64 CHashFNV1A::GetHash64()
{
    return mHash;
}

// ************ CONVENIENCE FUNCTIONS ************
void CHashFNV1A::HashByte(const u8& v)
{
    HashData(&v, 1);
}

void CHashFNV1A::HashShort(const u16& v)
{
    HashData(&v, 2);
}

void CHashFNV1A::HashLong(const u32& v)
{
    HashData(&v, 4);
}

void CHashFNV1A::HashFloat(const float& v)
{
    HashData(&v, 4);
}

void CHashFNV1A::HashString(const TString& v)
{
    HashData(v.Data(), v.Size());
}
