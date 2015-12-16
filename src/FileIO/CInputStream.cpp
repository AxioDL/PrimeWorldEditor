#include "CInputStream.h"

CInputStream::~CInputStream()
{
}

char CInputStream::ReadByte()
{
    char Val;
    ReadBytes(&Val, 1);
    return Val;
}

short CInputStream::ReadShort()
{
    short Val;
    ReadBytes(&Val, 2);
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

long CInputStream::ReadLong()
{
    long Val;
    ReadBytes(&Val, 4);
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

long long CInputStream::ReadLongLong()
{
    long long Val;
    ReadBytes(&Val, 8);
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

float CInputStream::ReadFloat()
{
    float Val;
    ReadBytes(&Val, 4);
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

double CInputStream::ReadDouble()
{
    double Val;
    ReadBytes(&Val, 8);
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    return Val;
}

std::string CInputStream::ReadString()
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

std::string CInputStream::ReadString(unsigned long Count)
{
    std::string Str(Count, 0);

    for (unsigned long c = 0; c < Count; c++)
        Str[c] = ReadByte();

    return Str;
}

std::wstring CInputStream::ReadWString()
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

std::wstring CInputStream::ReadWString(unsigned long Count)
{
    std::wstring WStr(Count, 0);

    for (unsigned long c = 0; c < Count; c++)
        WStr[c] = ReadShort();

    return WStr;
}


char CInputStream::PeekByte()
{
    char Val = ReadByte();
    Seek(-1, SEEK_CUR);
    return Val;
}

short CInputStream::PeekShort()
{
    short Val = ReadShort();
    Seek(-2, SEEK_CUR);
    return Val;
}

long CInputStream::PeekLong()
{
    long Val = ReadLong();
    Seek(-4, SEEK_CUR);
    return Val;
}

long long CInputStream::PeekLongLong()
{
    long long Val = ReadLongLong();
    Seek(-8, SEEK_CUR);
    return Val;
}

float CInputStream::PeekFloat()
{
    float Val = ReadFloat();
    Seek(-4, SEEK_CUR);
    return Val;
}

double CInputStream::PeekDouble()
{
    double Val = ReadDouble();
    Seek(-8, SEEK_CUR);
    return Val;
}

void CInputStream::SeekToBoundary(unsigned long Boundary)
{
    long Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    else Seek(Num, SEEK_CUR);
}

void CInputStream::SetEndianness(IOUtil::EEndianness Endianness)
{
    mDataEndianness = Endianness;
}

void CInputStream::SetSourceString(const std::string& source)
{
    mDataSource = source;
}

IOUtil::EEndianness CInputStream::GetEndianness() const
{
    return mDataEndianness;
}

std::string CInputStream::GetSourceString() const
{
    return mDataSource;
}

bool CInputStream::Seek64(long long Offset, long Origin)
{
    return Seek((long) Offset, Origin);
}

long long CInputStream::Tell64() const
{
    return (long long) Tell();
}
