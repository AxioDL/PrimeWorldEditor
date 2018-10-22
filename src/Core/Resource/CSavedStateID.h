#ifndef CSAVEDSTATEID_H
#define CSAVEDSTATEID_H

#include <Common/Common.h>

// GUID representing a value stored in the save file for MP3/DKCR
class CSavedStateID
{
    u64 m[2];

public:
    CSavedStateID()
    {
        m[0] = 0;
        m[1] = 0;
    }

    CSavedStateID(u64 Part1, u64 Part2)
    {
        m[0] = Part1;
        m[1] = Part2;
    }

    CSavedStateID(IInputStream& rInput)
    {
        m[0] = rInput.ReadLongLong();
        m[1] = rInput.ReadLongLong();
    }

    TString ToString()
    {
        u32 Part1 = (m[0] >> 32) & 0xFFFFFFFF;
        u32 Part2 = (m[0] >> 16) & 0x0000FFFF;
        u32 Part3 = (m[0] >> 00) & 0x0000FFFF;
        u32 Part4 = (m[1] >> 48) & 0x0000FFFF;
        u32 Part5 = (m[1] >> 32) & 0x0000FFFF;
        u32 Part6 = (m[1] >> 00) & 0xFFFFFFFF;
        return TString::Format("%08X-%04X-%04X-%04X-%04X%08X", Part1, Part2, Part3, Part4, Part5, Part6);
    }

    void Write(IOutputStream& rOutput)
    {
        rOutput.WriteLongLong(m[0]);
        rOutput.WriteLongLong(m[1]);
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
            if (rArc.IsWriter()) Str = ToString();
            rArc.SerializePrimitive(Str, 0);
            if (rArc.IsReader()) *this = FromString(Str);
        }
    }

    // Operators
    inline bool operator==(const CSavedStateID& rkOther)
    {
        return (m[0] == rkOther.m[0] && m[1] == rkOther.m[1]);
    }

    inline bool operator!=(const CSavedStateID& rkOther)
    {
        return !(*this == rkOther);
    }

    inline bool operator<(const CSavedStateID& rkOther)
    {
        return (m[0] == rkOther.m[0] ? m[1] < rkOther.m[1] : m[0] < rkOther.m[0]);
    }

    // Static
    static CSavedStateID FromString(TString Str)
    {
        Str.Remove('-');
        ASSERT(Str.Size() == 32);

        CSavedStateID Out;
        Out.m[0] = Str.SubString(0, 16).ToInt64(16);
        Out.m[1] = Str.SubString(16, 16).ToInt64(16);
        return Out;
    }

};

#endif // CSAVEDSTATEID_H
