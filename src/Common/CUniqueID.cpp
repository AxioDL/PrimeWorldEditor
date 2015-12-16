#include "CUniqueID.h"
#include "TString.h"

#include <iomanip>
#include <random>
#include <sstream>

// this class probably isn't optimized! this may not be the best way to do things
using IOUtil::kSystemEndianness;
using IOUtil::eLittleEndian;
using IOUtil::eBigEndian;

CUniqueID::CUniqueID()
{
    memset(mID, 0xFF, 16);
    mLength = eInvalidUIDLength;
}

CUniqueID::CUniqueID(u64 ID)
{
    // This constructor is intended to be used with both 32-bit and 64-bit input values
    memset(mID, 0xFF, 16);

    // 64-bit - check for valid content in upper 32 bits (at least one bit set + one bit unset)
    if ((ID & 0xFFFFFFFF00000000) && (~ID & 0xFFFFFFFF00000000))
    {
        memcpy(&mID, &ID, 8);
        mLength = e64Bit;
    }

    // 32-bit
    else
    {
        memcpy(mID, &ID, 4);
        mLength = e32Bit;
    }

    // Reverse for Big Endian
    if (kSystemEndianness == eBigEndian)
        Reverse();
}

CUniqueID::CUniqueID(u64 Part1, u64 Part2)
{
    // Constructor for 128-bit IDs
    memcpy(&mID[0], &Part1, 8);
    memcpy(&mID[8], &Part2, 8);
    mLength = e128Bit;
}

CUniqueID::CUniqueID(const char* ID)
{
    *this = CUniqueID::FromString(ID);
}

CUniqueID::CUniqueID(IInputStream& Input, EUIDLength Length)
{
    memset(mID, 0, 16);
    Input.ReadBytes(&mID[16 - Length], Length);

    if (Length != e128Bit)
        if (kSystemEndianness == eLittleEndian)
            Reverse();

    mLength = Length;
}

u32 CUniqueID::ToLong() const
{
    if (kSystemEndianness == eLittleEndian)
        return *((u32*) mID);
    else
        return *((u32*) &mID[12]);
}

u64 CUniqueID::ToLongLong() const
{
    if (kSystemEndianness == eLittleEndian)
        return *((u64*) mID);
    else
        return *((u64*) &mID[8]);
}

TString CUniqueID::ToString() const
{
    switch (mLength)
    {
    case e32Bit:
        return TString::FromInt32(ToLong(), 8);

    case e64Bit:
        return TString::FromInt64(ToLongLong(), 16);

    case e128Bit:
        // todo: TString should have a "FromInt128" function
        std::stringstream Ret;
        Ret << std::hex << std::setfill('0');

        for (u32 i = 0; i < 16; i++)
            Ret << std::setw(2) << (u32) mID[i];

        return Ret.str();
    }

    return "INVALID ID LENGTH";
}

void CUniqueID::Reverse()
{
    std::reverse(mID, &mID[16]);
}

EUIDLength CUniqueID::Length() const
{
    return mLength;
}

void CUniqueID::SetLength(EUIDLength Length)
{
    mLength = Length;
}

bool CUniqueID::IsValid() const
{
    if (mLength == e32Bit)
        return (*this != skInvalidID32);

    else if (mLength == e64Bit)
        return (*this != skInvalidID64);

    else if (mLength == e128Bit)
        return (*this != skInvalidID128);

    else return false;
}

// ************ OPERATORS ************
void CUniqueID::operator=(const u64& Input)
{
    *this = CUniqueID(Input);
}

void CUniqueID::operator=(const char* Input)
{
    *this = CUniqueID(Input);
}

bool CUniqueID::operator==(const CUniqueID& Other) const
{
    return ((mLength == Other.mLength) &&
            (memcmp(mID, Other.mID, 16) == 0));
}

bool CUniqueID::operator!=(const CUniqueID& Other) const
{
    return (!(*this == Other));
}

bool CUniqueID::operator>(const CUniqueID& Other) const
{
    if (mLength != Other.mLength)
        return mLength > Other.mLength;

    switch (mLength)
    {
    case e32Bit:
        return (ToLong() > Other.ToLong());

    case e64Bit:
        return (ToLongLong() > Other.ToLongLong());

    case e128Bit:
        for (u32 i = 0; i < 16; i++)
            if (mID[i] != Other.mID[i])
                return (mID[i] > Other.mID[i]);
        return false;

    default:
        return false;
    }
}

bool CUniqueID::operator>=(const CUniqueID& Other) const
{
    return ((*this == Other) || (*this > Other));
}

bool CUniqueID::operator<(const CUniqueID& Other) const
{
    if (mLength != Other.mLength)
        return mLength < Other.mLength;

    switch (mLength)
    {
    case e32Bit:
        return (ToLong() < Other.ToLong());

    case e64Bit:
        return (ToLongLong() < Other.ToLongLong());

    case e128Bit:
        for (u32 i = 0; i < 16; i++)
            if (mID[i] != Other.mID[i])
                return (mID[i] < Other.mID[i]);
        return false;

    default:
        return false;
    }
}

bool CUniqueID::operator<=(const CUniqueID& Other) const
{
    return ((*this == Other) || (*this < Other));
}

bool CUniqueID::operator==(u64 Other) const
{
    return (*this == CUniqueID(Other));
}

bool CUniqueID::operator!=(u64 Other) const
{
    return (!(*this == Other));
}

// ************ STATIC ************
CUniqueID CUniqueID::FromString(const TString& String)
{
    // If the input is a hex ID in string form, then preserve it... otherwise, generate an ID by hashing the string
    TString Name = String.GetFileName(false);
    u32 NameLength = Name.Length();

    if (Name.IsHexString())
    {
        if (NameLength == 8)
        {
            CUniqueID ID;
            ID.mLength = e32Bit;

            u32 LongID = Name.ToInt32();

            if (kSystemEndianness == eLittleEndian)
                memcpy(ID.mID, &LongID, 4);
            else
                memcpy(&ID.mID[12], &LongID, 4);

            return ID;
        }

        else if (NameLength == 16)
        {
            CUniqueID ID;
            ID.mLength = e64Bit;

            u64 LongID = Name.ToInt64();

            if (kSystemEndianness == eLittleEndian)
                memcpy(ID.mID, &LongID, 8);
            else
                memcpy(&ID.mID[8], &LongID, 8);

            return ID;
        }

        else if (NameLength == 32)
        {
            CUniqueID ID;
            ID.mLength = e128Bit;
            Name.ToInt128((char*) ID.mID);
            return ID;
        }
    }

    return CUniqueID(String.Hash64());
}

CUniqueID CUniqueID::FromData(void *pData, EUIDLength Length)
{
    CUniqueID ID;
    ID.mLength = Length;
    memcpy(ID.mID, pData, Length);
    return ID;
}

CUniqueID CUniqueID::RandomID()
{
    CUniqueID ID;
    ID.mLength = e128Bit;

    for (u32 i = 0; i < 16; i++)
        ID.mID[i] = rand() & 0xFF;

    return ID;
}

// ************ STATIC MEMBER INITIALIZATION ************
CUniqueID CUniqueID::skInvalidID32 = CUniqueID((u32) -1);
CUniqueID CUniqueID::skInvalidID64 = CUniqueID((u64) -1);
CUniqueID CUniqueID::skInvalidID128 = CUniqueID((u64) -1, (u64) -1);
