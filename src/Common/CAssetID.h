#ifndef CASSETID_H
#define CASSETID_H

#include "EGame.h"
#include "FileIO.h"
#include "TString.h"
#include "types.h"

enum EIDLength
{
    e32Bit = 4,
    e64Bit = 8,
    eInvalidIDLength = 0
};

class CAssetID
{
    EIDLength mLength;
    u64 mID;

public:
    CAssetID();
    CAssetID(u64 ID);
    CAssetID(u64 ID, EIDLength Length);
    CAssetID(IInputStream& rInput, EIDLength Length);
    CAssetID(IInputStream& rInput, EGame Game);
    void Write(IOutputStream& rOutput, EIDLength ForcedLength = eInvalidIDLength) const;
    TString ToString(EIDLength ForcedLength = eInvalidIDLength) const;
    bool IsValid() const;

    // Operators
    inline void operator= (const u64& rkInput)              { *this = CAssetID(rkInput); }
    inline bool operator==(const CAssetID& rkOther) const   { return mLength == rkOther.mLength && mID == rkOther.mID; }
    inline bool operator!=(const CAssetID& rkOther) const   { return mLength != rkOther.mLength || mID != rkOther.mID; }
    inline bool operator> (const CAssetID& rkOther) const   { return mLength >= rkOther.mLength && mID > rkOther.mID; }
    inline bool operator>=(const CAssetID& rkOther) const   { return mLength >= rkOther.mLength && mID >= rkOther.mID; }
    inline bool operator< (const CAssetID& rkOther) const   { return mLength <  rkOther.mLength || mID < rkOther.mID; }
    inline bool operator<=(const CAssetID& rkOther) const   { return mLength <  rkOther.mLength || mID <= rkOther.mID; }
    inline bool operator==(u64 Other) const                 { return mID == Other; }
    inline bool operator!=(u64 Other) const                 { return mID != Other; }

    // Accessors
    inline u32 ToLong() const               { return (u32) mID; }
    inline u64 ToLongLong() const           { return mID; }
    inline EIDLength Length() const         { return mLength; }
    inline void SetLength(EIDLength Length) { mLength = Length; }

    // Static
    static CAssetID FromString(const TString& rkString);
    static CAssetID RandomID();

    inline static EIDLength GameIDLength(EGame Game)        { return (Game == EGame::Invalid ? eInvalidIDLength : (Game <= EGame::Echoes ? e32Bit : e64Bit)); }
    inline static CAssetID InvalidID(EIDLength IDLength)    { return (IDLength == e32Bit ? skInvalidID32 : skInvalidID64); }
    inline static CAssetID InvalidID(EGame Game)            { return InvalidID(Game <= EGame::Echoes ? e32Bit : e64Bit); }

    static CAssetID skInvalidID32;
    static CAssetID skInvalidID64;
};

#endif // CASSETID_H
