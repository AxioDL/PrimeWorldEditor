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
    : mLength(eInvalidUIDLength)
{
    memset(mID, 0xFF, 16);
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

CUniqueID::CUniqueID(u64 ID, EUIDLength Length)
{
    // This constructor shouldn't be used for 128-bit
    memset(mID, 0xFF, 16);

    // 64-bit
    if (Length == e64Bit || Length == e128Bit)
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

CUniqueID::CUniqueID(const char* pkID)
{
    *this = CUniqueID::FromString(pkID);
}

CUniqueID::CUniqueID(IInputStream& rInput, EUIDLength Length)
{
    memset(mID, 0, 16);
    rInput.ReadBytes(&mID[16 - Length], Length);

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

        for (u32 iByte = 0; iByte < 16; iByte++)
            Ret << std::setw(2) << (u32) mID[iByte];

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
void CUniqueID::operator=(const u64& rkInput)
{
    *this = CUniqueID(rkInput);
}

void CUniqueID::operator=(const char* pkInput)
{
    *this = CUniqueID(pkInput);
}

bool CUniqueID::operator==(const CUniqueID& rkOther) const
{
    return ((mLength == rkOther.mLength) &&
            (memcmp(mID, rkOther.mID, 16) == 0));
}

bool CUniqueID::operator!=(const CUniqueID& rkOther) const
{
    return (!(*this == rkOther));
}

bool CUniqueID::operator>(const CUniqueID& rkOther) const
{
    if (mLength != rkOther.mLength)
        return mLength > rkOther.mLength;

    switch (mLength)
    {
    case e32Bit:
        return (ToLong() > rkOther.ToLong());

    case e64Bit:
        return (ToLongLong() > rkOther.ToLongLong());

    case e128Bit:
        for (u32 iByte = 0; iByte < 16; iByte++)
            if (mID[iByte] != rkOther.mID[iByte])
                return (mID[iByte] > rkOther.mID[iByte]);
        return false;

    default:
        return false;
    }
}

bool CUniqueID::operator>=(const CUniqueID& rkOther) const
{
    return ((*this == rkOther) || (*this > rkOther));
}

bool CUniqueID::operator<(const CUniqueID& rkOther) const
{
    if (mLength != rkOther.mLength)
        return mLength < rkOther.mLength;

    switch (mLength)
    {
    case e32Bit:
        return (ToLong() < rkOther.ToLong());

    case e64Bit:
        return (ToLongLong() < rkOther.ToLongLong());

    case e128Bit:
        for (u32 iByte = 0; iByte < 16; iByte++)
            if (mID[iByte] != rkOther.mID[iByte])
                return (mID[iByte] < rkOther.mID[iByte]);
        return false;

    default:
        return false;
    }
}

bool CUniqueID::operator<=(const CUniqueID& rkOther) const
{
    return ((*this == rkOther) || (*this < rkOther));
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
CUniqueID CUniqueID::FromString(const TString& rkString)
{
    // If the input is a hex ID in string form, then preserve it... otherwise, generate an ID by hashing the string
    TString Name = rkString.GetFileName(false);
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

    return CUniqueID(rkString.Hash64());
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

    for (u32 iByte = 0; iByte < 16; iByte++)
        ID.mID[iByte] = rand() & 0xFF;

    return ID;
}

// ************ STATIC MEMBER INITIALIZATION ************
CUniqueID CUniqueID::skInvalidID32 = CUniqueID((u32) -1, e32Bit);
CUniqueID CUniqueID::skInvalidID64 = CUniqueID((u64) -1, e64Bit);
CUniqueID CUniqueID::skInvalidID128 = CUniqueID((u64) -1, (u64) -1);
