#ifndef CSAVEDSTATEID_H
#define CSAVEDSTATEID_H

#include <Common/Common.h>
#include <array>

// GUID representing a value stored in the save file for MP3/DKCR
class CSavedStateID
{
    std::array<uint64, 2> m{};

public:
    constexpr CSavedStateID() = default;

    constexpr CSavedStateID(uint64 Part1, uint64 Part2) : m{{Part1, Part2}}
    {
    }

    explicit CSavedStateID(IInputStream& rInput) : m{rInput.ReadULongLong(), rInput.ReadULongLong()}
    {
    }

    TString ToString() const
    {
        const uint32 Part1 = (m[0] >> 32) & 0xFFFFFFFF;
        const uint32 Part2 = (m[0] >> 16) & 0x0000FFFF;
        const uint32 Part3 = (m[0] >> 00) & 0x0000FFFF;
        const uint32 Part4 = (m[1] >> 48) & 0x0000FFFF;
        const uint32 Part5 = (m[1] >> 32) & 0x0000FFFF;
        const uint32 Part6 = (m[1] >> 00) & 0xFFFFFFFF;
        return TString::Format("%08X-%04X-%04X-%04X-%04X%08X", Part1, Part2, Part3, Part4, Part5, Part6);
    }

    void Write(IOutputStream& rOutput)
    {
        rOutput.WriteULongLong(m[0]);
        rOutput.WriteULongLong(m[1]);
    }

    void Serialize(IArchive& rArc)
    {
        if (rArc.IsBinaryFormat())
        {
            rArc.SerializePrimitive(m[0], 0);
            rArc.SerializePrimitive(m[1], 0);
        }
        else
        {
            TString Str;
            if (rArc.IsWriter())
                Str = ToString();
            rArc.SerializePrimitive(Str, 0);
            if (rArc.IsReader())
                *this = FromString(Str);
        }
    }

    // Operators
    bool operator==(const CSavedStateID& rkOther) const
    {
        return m == rkOther.m;
    }

    bool operator!=(const CSavedStateID& rkOther) const
    {
        return !operator==(rkOther);
    }

    bool operator<(const CSavedStateID& rkOther) const
    {
        return (m[0] == rkOther.m[0] ? m[1] < rkOther.m[1] : m[0] < rkOther.m[0]);
    }

    // Static
    static CSavedStateID FromString(TString Str)
    {
        Str.Remove('-');
        ASSERT(Str.Size() == 32);

        CSavedStateID Out;
        Str.ToInt128(&Out.m[0]);
        return Out;
    }

};

#endif // CSAVEDSTATEID_H
