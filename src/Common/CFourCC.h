#ifndef CFOURCC_H
#define CFOURCC_H

#include "types.h"
#include "TString.h"
#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CFourCC
{
    char mFourCC[4];
public:
    // Constructors
    CFourCC()                       { memset(mFourCC, 0, 4); }
    CFourCC(const char *pkSrc)      { *this = pkSrc; }
    CFourCC(const TString& rkSrc)   { *this = rkSrc; }
    CFourCC(u32 Src)                { *this = Src; }
    CFourCC(IInputStream& rSrc)     { rSrc.ReadBytes(&mFourCC[0], 4); }

    // Functionality
    inline void Write(IOutputStream& rOutput)
    {
        rOutput.WriteBytes(&mFourCC[0], 4);
    }

    inline u32 ToLong() const
    {
        return mFourCC[0] << 24 | mFourCC[1] << 16 | mFourCC[2] << 8 | mFourCC[3];
    }

    inline TString ToString() const
    {
        return TString(mFourCC, 4);
    }

    inline CFourCC ToUpper() const
    {
        CFourCC Out;

        for (int iChr = 0; iChr < 4; iChr++)
        {
            if ((mFourCC[iChr] >= 0x61) && (mFourCC[iChr] <= 0x7A))
                Out.mFourCC[iChr] = mFourCC[iChr] - 0x20;
            else
                Out.mFourCC[iChr] = mFourCC[iChr];
        }

        return Out;
    }

    // Operators
    inline CFourCC& operator=(const char *pkSrc)
    {
        memcpy(&mFourCC[0], pkSrc, 4);
        return *this;
    }

    inline CFourCC& operator=(const TString& rkSrc)
    {
        memcpy(&mFourCC[0], rkSrc.CString(), 4);
        return *this;
    }

    inline CFourCC& operator=(u32 Src)
    {
        mFourCC[0] = (Src >> 24) & 0xFF;
        mFourCC[1] = (Src >> 16) & 0xFF;
        mFourCC[2] = (Src >>  8) & 0xFF;
        mFourCC[3] = (Src >>  0) & 0xFF;
        return *this;
    }

    inline bool operator==(const CFourCC& rkOther) const    { return ((mFourCC[0] == rkOther.mFourCC[0]) && (mFourCC[1] == rkOther.mFourCC[1]) && (mFourCC[2] == rkOther.mFourCC[2]) && (mFourCC[3] == rkOther.mFourCC[3])); }
    inline bool operator!=(const CFourCC& rkOther) const    { return (!(*this == rkOther)); }
    inline bool operator>(const CFourCC& rkOther) const     { return (ToLong() > rkOther.ToLong()); }
    inline bool operator>=(const CFourCC& rkOther) const    { return (ToLong() >= rkOther.ToLong()); }
    inline bool operator<(const CFourCC& rkOther) const     { return (ToLong() < rkOther.ToLong()); }
    inline bool operator<=(const CFourCC& rkOther) const    { return (ToLong() <= rkOther.ToLong()); }
    inline char operator[](int Index)                       { return mFourCC[Index]; }
    inline const char operator[](int Index) const           { return mFourCC[Index]; }
};

#endif // CFOURCC_H
