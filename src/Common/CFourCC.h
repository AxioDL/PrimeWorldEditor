#ifndef CFOURCC_H
#define CFOURCC_H

#include "AssertMacro.h"
#include "FileIO.h"
#include "types.h"
#include "TString.h"

#define FOURCC_FROM_TEXT(Text) (Text[0] << 24 | Text[1] << 16 | Text[2] << 8 | Text[3])

// Note: All FourCC constants should be wrapped in this macro
#define FOURCC(Value) Value

class CFourCC
{
    // Note: mFourCC_Chars isn't used much due to endianness.
    union
    {
        u32 mFourCC;
        char mFourCC_Chars[4];
    };

public:
    // Constructors
    inline CFourCC()                        { mFourCC = 0; }
    inline CFourCC(const char *pkSrc)       { mFourCC = FOURCC_FROM_TEXT(pkSrc); }
    inline CFourCC(const TString& rkSrc)    { ASSERT(rkSrc.Length() == 4); mFourCC = FOURCC_FROM_TEXT(rkSrc); }
    inline CFourCC(const TWideString& rkSrc){ ASSERT(rkSrc.Length() == 4); mFourCC = FOURCC_FROM_TEXT(rkSrc); }
    inline CFourCC(u32 Src)                 { mFourCC = Src; }
    inline CFourCC(IInputStream& rSrc)      { Read(rSrc); }

    // Functionality
    inline void Read(IInputStream& rInput)
    {
        mFourCC = rInput.ReadLong();
        if (rInput.GetEndianness() == IOUtil::eLittleEndian) Reverse();
    }

    inline void Write(IOutputStream& rOutput) const
    {
        u32 Val = mFourCC;
        if (rOutput.GetEndianness() == IOUtil::eLittleEndian) IOUtil::SwapBytes(Val);
        rOutput.WriteLong(Val);
    }

    inline u32 ToLong() const
    {
        return mFourCC;
    }

    inline TString ToString() const
    {
        char CharArray[4] = {
            (char) ((mFourCC >> 24) & 0xFF),
            (char) ((mFourCC >> 16) & 0xFF),
            (char) ((mFourCC >>  8) & 0xFF),
            (char) ((mFourCC >>  0) & 0xFF)
        };

        return TString(CharArray, 4);
    }

    inline CFourCC ToUpper() const
    {
        CFourCC Out;

        for (int iChr = 0; iChr < 4; iChr++)
            Out.mFourCC_Chars[iChr] = TString::CharToUpper(mFourCC_Chars[iChr]);

        return CFourCC(Out);
    }

    inline void Reverse() const
    {
        IOUtil::SwapBytes((u32&) mFourCC);
    }

    // Operators
    inline char& operator[](int Index)
    {
        ASSERT(Index >= 0 && Index < 4);
        if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian)
            Index = 3 - Index;

        return ((char*)(&mFourCC))[Index];
    }

    inline const char& operator[](int Index) const
    {
        ASSERT(Index >= 0 && Index < 4);
        if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian)
            Index = 3 - Index;

        return ((char*)(&mFourCC))[Index];
    }

    inline TString operator+(const char *pkText) const
    {
        return ToString() + pkText;
    }

    inline TString operator+(const TString& rkStr) const
    {
        return ToString() + rkStr;
    }

    inline friend TString operator+(const char *pkText, const CFourCC& rkFourCC)
    {
        return pkText + rkFourCC.ToString();
    }

    inline friend TString operator+(const TString& rkStr, const CFourCC& rkFourCC)
    {
        return rkStr + rkFourCC.ToString();
    }

    inline CFourCC& operator=(const char *pkSrc)            { mFourCC = FOURCC_FROM_TEXT(pkSrc);    return *this;   }
    inline CFourCC& operator=(const TString& rkSrc)         { mFourCC = FOURCC_FROM_TEXT(rkSrc);    return *this;   }
    inline CFourCC& operator=(u32 Src)                      { mFourCC = Src;                        return *this;   }
    inline bool operator==(const CFourCC& rkOther) const    { return mFourCC == rkOther.mFourCC;                    }
    inline bool operator!=(const CFourCC& rkOther) const    { return mFourCC != rkOther.mFourCC;                    }
    inline bool operator> (const CFourCC& rkOther) const    { return mFourCC >  rkOther.mFourCC;                    }
    inline bool operator>=(const CFourCC& rkOther) const    { return mFourCC >= rkOther.mFourCC;                    }
    inline bool operator< (const CFourCC& rkOther) const    { return mFourCC <  rkOther.mFourCC;                    }
    inline bool operator<=(const CFourCC& rkOther) const    { return mFourCC <= rkOther.mFourCC;                    }
};

#endif // CFOURCC_H
