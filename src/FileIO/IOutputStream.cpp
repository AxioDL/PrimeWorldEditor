#include "IOutputStream.h"

IOutputStream::~IOutputStream()
{
}

void IOutputStream::WriteByte(char Val)
{
    WriteBytes(&Val, 1);
}

void IOutputStream::WriteShort(short Val)
{
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 2);
}

void IOutputStream::WriteLong(long Val)
{
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 4);
}

void IOutputStream::WriteLongLong(long long Val)
{
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 8);
}

void IOutputStream::WriteFloat(float Val)
{
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 4);
}

void IOutputStream::WriteDouble(double Val)
{
    if (mDataEndianness != IOUtil::kSystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 8);
}

void IOutputStream::WriteString(const std::string& rkVal)
{
    for (unsigned int i = 0; i < rkVal.size(); i++)
        WriteByte(rkVal[i]);

    if ((rkVal.empty()) || (rkVal.back() != '\0'))
        WriteByte(0);
}

void IOutputStream::WriteString(const std::string& rkVal, unsigned long Count, bool Terminate)
{
    for (unsigned int i = 0; i < Count; i++)
        WriteByte(rkVal[i]);

    if (Terminate && (rkVal[Count-1] != '\0'))
        WriteByte(0);
}

void IOutputStream::WriteWideString(const std::wstring& rkVal)
{
    for (unsigned int i = 0; i < rkVal.size(); i++)
        WriteShort(rkVal[i]);

    if ((!rkVal.empty()) && (rkVal.back() != '\0'))
        WriteShort(0);
}

void IOutputStream::WriteWideString(const std::wstring& rkVal, unsigned long Count, bool Terminate)
{
    for (unsigned int i = 0; i < Count; i++)
        WriteShort(rkVal[i]);

    if (Terminate && (rkVal[Count-1] != 0))
        WriteShort(0);
}

void IOutputStream::WriteToBoundary(unsigned long Boundary, char Fill)
{
    long Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    for (int i = 0; i < Num; i++)
        WriteByte(Fill);
}

void IOutputStream::SetEndianness(IOUtil::EEndianness Endianness)
{
    mDataEndianness = Endianness;
}

void IOutputStream::SetDestString(const std::string& rkDest)
{
    mDataDest = rkDest;
}

IOUtil::EEndianness IOutputStream::GetEndianness() const
{
    return mDataEndianness;
}

std::string IOutputStream::GetDestString() const
{
    return mDataDest;
}

bool IOutputStream::Seek64(long long Offset, long Origin)
{
    return Seek((long) Offset, Origin);
}

long long IOutputStream::Tell64() const
{
    return (long long) (Tell());
}
