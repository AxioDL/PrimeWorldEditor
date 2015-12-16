#include "COutputStream.h"

COutputStream::~COutputStream()
{
}

void COutputStream::WriteByte(char Val)
{
    WriteBytes(&Val, 1);
}

void COutputStream::WriteShort(short Val)
{
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 2);
}

void COutputStream::WriteLong(long Val)
{
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 4);
}

void COutputStream::WriteLongLong(long long Val)
{
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 8);
}

void COutputStream::WriteFloat(float Val)
{
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 4);
}

void COutputStream::WriteDouble(double Val)
{
    if (mDataEndianness != IOUtil::SystemEndianness) IOUtil::SwapBytes(Val);
    WriteBytes(&Val, 8);
}

void COutputStream::WriteString(std::string Val)
{
    for (unsigned int i = 0; i < Val.size(); i++)
        WriteByte(Val[i]);

    if ((Val.empty()) || (Val.back() != '\0'))
        WriteByte(0);
}

void COutputStream::WriteString(std::string Val, unsigned long Count, bool Terminate)
{
    for (unsigned int i = 0; i < Count; i++)
        WriteByte(Val[i]);

    if (Terminate && (Val[Count-1] != '\0'))
        WriteByte(0);
}

void COutputStream::WriteWString(std::wstring Val)
{
    for (unsigned int i = 0; i < Val.size(); i++)
        WriteShort(Val[i]);

    if ((!Val.empty()) && (Val.back() != '\0'))
        WriteShort(0);
}

void COutputStream::WriteWString(std::wstring Val, unsigned long Count, bool Terminate)
{
    for (unsigned int i = 0; i < Count; i++)
        WriteShort(Val[i]);

    if (Terminate && (Val[Count-1] != 0))
        WriteShort(0);
}

void COutputStream::WriteToBoundary(unsigned long Boundary, char Fill)
{
    long Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    for (int i = 0; i < Num; i++)
        WriteByte(Fill);
}

void COutputStream::SetEndianness(IOUtil::EEndianness Endianness)
{
    mDataEndianness = Endianness;
}

void COutputStream::SetDestString(const std::string &Dest)
{
    mDataDest = Dest;
}

IOUtil::EEndianness COutputStream::GetEndianness() const
{
    return mDataEndianness;
}

std::string COutputStream::GetDestString() const
{
    return mDataDest;
}

bool COutputStream::Seek64(long long Offset, long Origin)
{
    return Seek((long) Offset, Origin);
}

long long COutputStream::Tell64() const
{
    return (long long) (Tell());
}
