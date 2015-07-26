#include "CFourCC.h"

CFourCC::CFourCC()
{
    FourCC[0] = 0;
    FourCC[1] = 0;
    FourCC[2] = 0;
    FourCC[3] = 0;
}

CFourCC::CFourCC(const char *src)
{
    *this = src;
}

CFourCC::CFourCC(const std::string& src)
{
    *this = src;
}

CFourCC::CFourCC(long src)
{
    *this = src;
}

CFourCC::CFourCC(CInputStream& src)
{
    src.ReadBytes(&FourCC[0], 4);
}

void CFourCC::Write(COutputStream &Output)
{
    Output.WriteBytes(FourCC, 4);
}

CFourCC& CFourCC::operator=(const char *src)
{

    memcpy(&FourCC[0], src, 4);
    return *this;
}

CFourCC& CFourCC::operator=(const std::string& src)
{
    memcpy(&FourCC[0], src.c_str(), 4);
    return *this;
}

CFourCC& CFourCC::operator=(long src)
{
    FourCC[0] = (src >> 24) & 0xFF;
    FourCC[1] = (src >> 16) & 0xFF;
    FourCC[2] = (src >>  8) & 0xFF;
    FourCC[3] = (src >>  0) & 0xFF;
    return *this;
}

bool CFourCC::operator==(const CFourCC& other) const
{
    return ((FourCC[0] == other.FourCC[0]) && (FourCC[1] == other.FourCC[1]) && (FourCC[2] == other.FourCC[2]) && (FourCC[3] == other.FourCC[3]));
}

bool CFourCC::operator!=(const CFourCC& other) const
{
    return (!(*this == other));
}

bool CFourCC::operator==(const char *other) const
{
    return (*this == CFourCC(other));
}

bool CFourCC::operator!=(const char *other) const
{
    return (!(*this == other));
}

bool CFourCC::operator==(const long other) const
{
    return (*this == CFourCC(other));
}

bool CFourCC::operator!=(const long other) const
{
    return (!(*this == other));
}

long CFourCC::ToLong() const
{
    return FourCC[0] << 24 | FourCC[1] << 16 | FourCC[2] << 8 | FourCC[3];
}

std::string CFourCC::ToString() const
{
    return std::string(FourCC, 4);
}

CFourCC CFourCC::ToUpper() const
{
    CFourCC Out;

    for (int c = 0; c < 4; c++)
    {
        if ((FourCC[c] >= 0x61) && (FourCC[c] <= 0x7A))
            Out.FourCC[c] = FourCC[c] - 0x20;
        else
            Out.FourCC[c] = FourCC[c];
    }

    return Out;
}
