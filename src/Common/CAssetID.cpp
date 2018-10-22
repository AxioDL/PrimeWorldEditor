#include "CAssetID.h"
#include "TString.h"
#include <random>

CAssetID::CAssetID()
    : mLength(eInvalidIDLength)
    , mID(0xFFFFFFFFFFFFFFFF)
{

}

CAssetID::CAssetID(u64 ID)
    : mID(ID)
{
    // This constructor is intended to be used with both 32-bit and 64-bit input values
    // 64-bit - check for valid content in upper 32 bits (at least one bit set + one bit unset)
    if ((ID & 0xFFFFFFFF00000000) && (~ID & 0xFFFFFFFF00000000))
        mLength = e64Bit;

    // 32-bit
    else
    {
        mLength = e32Bit;
        mID &= 0xFFFFFFFF;
    }
}

CAssetID::CAssetID(u64 ID, EIDLength Length)
    : mID(ID)
    , mLength(Length)
{
    if (Length == e32Bit)
        mID &= 0xFFFFFFFF;
}

void CAssetID::Write(IOutputStream& rOutput, EIDLength ForcedLength /*= eInvalidIDLength*/) const
{
    EIDLength Length = (ForcedLength == eInvalidIDLength ? mLength : ForcedLength);

    if (Length == e32Bit)
        rOutput.WriteLong(ToLong());
    else
        rOutput.WriteLongLong(ToLongLong());
}

CAssetID::CAssetID(IInputStream& rInput, EIDLength Length)
    : mLength(Length)
{
    if (Length == e32Bit)   mID = ((u64) rInput.ReadLong()) & 0xFFFFFFFF;
    else                    mID = rInput.ReadLongLong();
}

CAssetID::CAssetID(IInputStream& rInput, EGame Game)
{
    *this = CAssetID(rInput, (Game <= EGame::Echoes ? e32Bit : e64Bit));
}

TString CAssetID::ToString(EIDLength ForcedLength /*= eInvalidIDLength*/) const
{
    EIDLength Length = (ForcedLength == eInvalidIDLength ? mLength : ForcedLength);

    if (Length == e32Bit)
        return TString::HexString(ToLong(), 8, false, true);
    else
        return TString::FromInt64(ToLongLong(), 16, 16).ToUpper();
}

bool CAssetID::IsValid() const
{
    return (mID != 0 && mLength != eInvalidIDLength && mID != InvalidID(mLength).mID);
}

// ************ STATIC ************
CAssetID CAssetID::FromString(const TString& rkString)
{
    // If the input is a hex ID in string form, then preserve it... otherwise, generate an ID by hashing the string
    TString Name = rkString.GetFileName(false);
    u32 NameLength = Name.Length();

    if (Name.IsHexString())
    {
        if (NameLength == 8)  return CAssetID(Name.ToInt32());
        if (NameLength == 16) return CAssetID(Name.ToInt64());
    }

    return CAssetID(rkString.Hash64());
}

CAssetID CAssetID::RandomID()
{
    CAssetID ID;
    ID.mLength = e64Bit;
    ID.mID = (u64(rand()) << 32) | rand();
    return ID;
}

// ************ STATIC MEMBER INITIALIZATION ************
CAssetID CAssetID::skInvalidID32 = CAssetID((u64) -1, e32Bit);
CAssetID CAssetID::skInvalidID64 = CAssetID((u64) -1, e64Bit);
