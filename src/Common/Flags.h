#ifndef FLAGS_H
#define FLAGS_H

#include "types.h"
#include "Common/Serialization/IArchive.h"

template<typename FlagEnum>
class TFlags
{
    u32 mValue;

public:
    TFlags() : mValue(0) {}
    TFlags(int Val) : mValue(Val) {}
    TFlags(u32 Val) : mValue(Val) {}
    TFlags(FlagEnum Val) : mValue(Val) {}

    inline operator   int()   const { return mValue;  }
    inline bool operator!()   const { return !mValue; }
    inline TFlags operator~() const { return TFlags(FlagEnum(~mValue)); }

    inline void operator&=(int Mask)        { mValue &= Mask; }
    inline void operator&=(u32 Mask)        { mValue &= Mask; }
    inline void operator|=(TFlags Flags)    { mValue |= Flags.mValue; }
    inline void operator|=(FlagEnum Flag)   { mValue |= Flag; }

    inline TFlags operator|(TFlags Flags) const     { return TFlags(FlagEnum(mValue | Flags.mValue)); }
    inline TFlags operator|(FlagEnum Flag) const    { return TFlags(FlagEnum(mValue | Flag)); }
    inline TFlags operator&(int Mask) const         { return TFlags(FlagEnum(mValue & Mask)); }
    inline TFlags operator&(u32 Mask) const         { return TFlags(FlagEnum(mValue & Mask)); }
    inline TFlags operator&(FlagEnum Flag) const    { return TFlags(FlagEnum(mValue & Flag)); }

    inline bool HasFlag(FlagEnum Flag) const    { return ((mValue & Flag) != 0); }
    inline bool HasAnyFlags(TFlags Flags) const { return ((mValue & Flags) != 0); }
    inline bool HasAllFlags(TFlags Flags) const { return ((mValue & Flags) == Flags); }
    inline void SetFlag(FlagEnum Flag)          { mValue |= Flag; }
    inline void SetFlag(TFlags Flags)           { mValue |= Flags; }
    inline void ClearFlag(FlagEnum Flag)        { mValue &= ~Flag; }
    inline void ClearFlag(TFlags Flags)         { mValue &= ~Flags; }

    inline void Serialize(IArchive& rArc)       { rArc.SerializeHexPrimitive(mValue); }
};
#define DECLARE_FLAGS(Enum, FlagTypeName) typedef TFlags<Enum> FlagTypeName;

#endif // FLAGS_H

