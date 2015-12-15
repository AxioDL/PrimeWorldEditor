#include "CFourCC.h"

// ************ CONSTRUCTORS ************
CFourCC::CFourCC()
{
    memset(mFourCC, 0, 4);
}

CFourCC::CFourCC(const char *src)
{
    *this = src;
}

CFourCC::CFourCC(const TString& src)
{
    *this = src;
}

CFourCC::CFourCC(u32 src)
{
    *this = src;
}

CFourCC::CFourCC(CInputStream& src)
{
    src.ReadBytes(&mFourCC[0], 4);
}

// ************ FUNCTIONALITY ************
void CFourCC::Write(COutputStream &Output)
{
    Output.WriteBytes(mFourCC, 4);
}

u32 CFourCC::ToLong() const
{
    return mFourCC[0] << 24 | mFourCC[1] << 16 | mFourCC[2] << 8 | mFourCC[3];
}

TString CFourCC::ToString() const
{
    return TString(mFourCC, 4);
}

CFourCC CFourCC::ToUpper() const
{
    CFourCC Out;

    for (int c = 0; c < 4; c++)
    {
        if ((mFourCC[c] >= 0x61) && (mFourCC[c] <= 0x7A))
            Out.mFourCC[c] = mFourCC[c] - 0x20;
        else
            Out.mFourCC[c] = mFourCC[c];
    }

    return Out;
}

// ************ OPERATORS ************
CFourCC& CFourCC::operator=(const char *src)
{
    memcpy(&mFourCC[0], src, 4);
    return *this;
}

CFourCC& CFourCC::operator=(const TString& src)
{
    memcpy(&mFourCC[0], src.CString(), 4);
    return *this;
}

CFourCC& CFourCC::operator=(u32 src)
{
    mFourCC[0] = (src >> 24) & 0xFF;
    mFourCC[1] = (src >> 16) & 0xFF;
    mFourCC[2] = (src >>  8) & 0xFF;
    mFourCC[3] = (src >>  0) & 0xFF;
    return *this;
}

bool CFourCC::operator==(const CFourCC& other) const
{
    return ((mFourCC[0] == other.mFourCC[0]) && (mFourCC[1] == other.mFourCC[1]) && (mFourCC[2] == other.mFourCC[2]) && (mFourCC[3] == other.mFourCC[3]));
}

bool CFourCC::operator!=(const CFourCC& other) const
{
    return (!(*this == other));
}

bool CFourCC::operator>(const CFourCC& other) const
{
    return (ToLong() > other.ToLong());
}

bool CFourCC::operator>=(const CFourCC& other) const
{
    return (ToLong() >= other.ToLong());
}

bool CFourCC::operator<(const CFourCC& other) const
{
    return (ToLong() < other.ToLong());
}

bool CFourCC::operator<=(const CFourCC& other) const
{
    return (ToLong() <= other.ToLong());
}

char CFourCC::operator[](int index)
{
    return mFourCC[index];
}

const char CFourCC::operator[](int index) const
{
    return mFourCC[index];
}
