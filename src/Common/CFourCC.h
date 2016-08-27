#ifndef CFOURCC_H
#define CFOURCC_H

#include "AssertMacro.h"
#include "types.h"
#include "TString.h"
#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

#define FOURCC(Text) (Text[0] << 24 | Text[1] << 16 | Text[2] << 8 | Text[3])
#define FOURCC_CONSTEXPR(A, B, C, D) (A << 24 | B << 16 | C << 8 | D)

class CFourCC
{
    // Note: mFourCC_Chars isn't really used due to endian issues. It's mostly here for easier readability in the debugger.
    union
    {
        u32 mFourCC;
        char mFourCC_Chars[4];
    };

public:
    // Constructors
    inline CFourCC()                        { mFourCC = 0; }
    inline CFourCC(const char *pkSrc)       { mFourCC = FOURCC(pkSrc); }
    inline CFourCC(const TString& rkSrc)    { ASSERT(rkSrc.Length() == 4); mFourCC = FOURCC(rkSrc); }
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
            ( (mFourCC >> 24) & 0xFF),
            ( (mFourCC >> 16) & 0xFF),
            ( (mFourCC >>  8) & 0xFF),
            ( (mFourCC >>  0) & 0xFF)
        };

        return TString(CharArray, 4);
    }

    inline CFourCC ToUpper() const
    {
        CFourCC Out;

        for (int iChr = 0; iChr < 4; iChr++)
        {
            char Chr = (*this)[iChr];

            if ((Chr >= 0x61) && (Chr <= 0x7A))
                Chr -= 0x20;

            Out.mFourCC |= (Chr << (8 * (3 - iChr)));
        }

        return CFourCC(Out);
    }

    inline void Reverse() const
    {
        IOUtil::SwapBytes((u32) mFourCC);
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

    inline CFourCC& operator=(const char *pkSrc)            { mFourCC = FOURCC(pkSrc);  return *this;   }
    inline CFourCC& operator=(const TString& rkSrc)         { mFourCC = FOURCC(rkSrc);  return *this;   }
    inline CFourCC& operator=(u32 Src)                      { mFourCC = Src;            return *this;   }
    inline bool operator==(const CFourCC& rkOther) const    { return mFourCC == rkOther.mFourCC;        }
    inline bool operator!=(const CFourCC& rkOther) const    { return mFourCC != rkOther.mFourCC;        }
    inline bool operator> (const CFourCC& rkOther) const    { return mFourCC >  rkOther.mFourCC;        }
    inline bool operator>=(const CFourCC& rkOther) const    { return mFourCC >= rkOther.mFourCC;        }
    inline bool operator< (const CFourCC& rkOther) const    { return mFourCC <  rkOther.mFourCC;        }
    inline bool operator<=(const CFourCC& rkOther) const    { return mFourCC <= rkOther.mFourCC;        }
};

#endif // CFOURCC_H
