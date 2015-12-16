#include "IInputStream.h"

IInputStream::~IInputStream()
{
}

char IInputStream::ReadByte()
{
    char Val;
    ReadBytes(&Val, 1);
    return Val;
}

short IInputStream::ReadShort()
{
    short Val;
    ReadBytes(&Val, 2);
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

long IInputStream::ReadLong()
{
    long Val;
    ReadBytes(&Val, 4);
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

long long IInputStream::ReadLongLong()
{
    long long Val;
    ReadBytes(&Val, 8);
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

float IInputStream::ReadFloat()
{
    float Val;
    ReadBytes(&Val, 4);
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

double IInputStream::ReadDouble()
{
    double Val;
    ReadBytes(&Val, 8);
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

std::string IInputStream::ReadString()
{
    std::string Str;
    char c = 1;

    while ((c != 0) && (!EoF()))
    {
        c = ReadByte();
        if (c != 0) Str.push_back(c);
    }

    return Str;
}

std::string IInputStream::ReadString(unsigned long Count)
{
    std::string Str(Count, 0);

    for (unsigned long c = 0; c < Count; c++)
        Str[c] = ReadByte();

    return Str;
}

std::wstring IInputStream::ReadWString()
{
    std::wstring WStr;
    short c = 1;

    while (c != 0)
    {
        c = ReadShort();
        if (c != 0) WStr.push_back(c);
    }

    return WStr;
}

std::wstring IInputStream::ReadWString(unsigned long Count)
{
    std::wstring WStr(Count, 0);

    for (unsigned long c = 0; c < Count; c++)
        WStr[c] = ReadShort();

    return WStr;
}


char IInputStream::PeekByte()
{
    char Val = ReadByte();
    Seek(-1, SEEK_CUR);
    return Val;
}

short IInputStream::PeekShort()
{
    short Val = ReadShort();
    Seek(-2, SEEK_CUR);
    return Val;
}

long IInputStream::PeekLong()
{
    long Val = ReadLong();
    Seek(-4, SEEK_CUR);
    return Val;
}

long long IInputStream::PeekLongLong()
{
    long long Val = ReadLongLong();
    Seek(-8, SEEK_CUR);
    return Val;
}

float IInputStream::PeekFloat()
{
    float Val = ReadFloat();
    Seek(-4, SEEK_CUR);
    return Val;
}

double IInputStream::PeekDouble()
{
    double Val = ReadDouble();
    Seek(-8, SEEK_CUR);
    return Val;
}

void IInputStream::SeekToBoundary(unsigned long Boundary)
{
    long Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    else Seek(Num, SEEK_CUR);
}

void IInputStream::SetEndianness(IOUtil::EEndianness Endianness)
{
    mDataEndianness = Endianness;
}

void IInputStream::SetSourceString(const std::string& rkSource)
{
    mDataSource = rkSource;
}

IOUtil::EEndianness IInputStream::GetEndianness() const
{
    return mDataEndianness;
}

std::string IInputStream::GetSourceString() const
{
    return mDataSource;
}

bool IInputStream::Seek64(long long Offset, long Origin)
{
    return Seek((long) Offset, Origin);
}

long long IInputStream::Tell64() const
{
    return (long long) Tell();
}
