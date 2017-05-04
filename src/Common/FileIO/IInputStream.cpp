#include "IInputStream.h"
#include <assert.h>

IInputStream::~IInputStream()
{
}

bool IInputStream::ReadBool()
{
    char Val;
    ReadBytes(&Val, 1);
    assert(Val == 0 || Val == 1);
    return (Val != 0 ? true : false);
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

long IInputStream::ReadFourCC()
{
    long Val;
    ReadBytes(&Val, 4);
    if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian) IOUtil::SwapBytes(Val);
    return Val;
}

TString IInputStream::ReadString()
{
    TString Str;
    char Chr;

    do
    {
        Chr = ReadByte();
        if (Chr != 0) Str.Append(Chr);
    }
    while ((Chr != 0) && (!EoF()));

    return Str;
}

TString IInputStream::ReadString(u32 Count)
{
    TString Str(Count, 0);
    ReadBytes(&Str[0], Count);
    return Str;
}

TString IInputStream::ReadSizedString()
{
    u32 StringSize = ReadLong();
    return ReadString(StringSize);
}

TWideString IInputStream::ReadWString()
{
    TWideString WStr;
    short Chr = 1;

    do
    {
        Chr = ReadShort();
        if (Chr != 0) WStr.Append(Chr);
    }
    while (Chr != 0 && !EoF());

    return WStr;
}

TWideString IInputStream::ReadWString(u32 Count)
{
    TWideString WStr(Count, 0);
    ReadBytes(&WStr[0], WStr.Size() * 2);
    return WStr;
}

TWideString IInputStream::ReadSizedWString()
{
    u32 StringSize = ReadLong();
    return ReadWString(StringSize);
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

long IInputStream::PeekFourCC()
{
    long Val = ReadFourCC();
    Seek(-4, SEEK_CUR);
    return Val;
}

bool IInputStream::GoTo(u32 Address)
{
    return Seek(Address, SEEK_SET);
}

bool IInputStream::Skip(s32 SkipAmount)
{
    return Seek(SkipAmount, SEEK_CUR);
}

void IInputStream::SeekToBoundary(u32 Boundary)
{
    u32 Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    else Seek(Num, SEEK_CUR);
}

void IInputStream::SetEndianness(IOUtil::EEndianness Endianness)
{
    mDataEndianness = Endianness;
}

void IInputStream::SetSourceString(const TString& rkSource)
{
    mDataSource = rkSource;
}

IOUtil::EEndianness IInputStream::GetEndianness() const
{
    return mDataEndianness;
}

TString IInputStream::GetSourceString() const
{
    return mDataSource;
}

bool IInputStream::Seek64(s64 Offset, u32 Origin)
{
    return Seek((s32) Offset, Origin);
}

u64 IInputStream::Tell64() const
{
    return (u64) Tell();
}
