#include "IOutputStream.h"

IOutputStream::~IOutputStream()
{
}

void IOutputStream::WriteBool(bool Val)
{
    char ChrVal = (Val ? 1 : 0);
    WriteBytes(&ChrVal, 1);
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
    for (unsigned int iChr = 0; iChr < Count; iChr++)
        WriteByte(rkVal[iChr]);

    if (Terminate && (rkVal[Count-1] != '\0'))
        WriteByte(0);
}

void IOutputStream::WriteSizedString(const std::string& rkVal)
{
    WriteLong(rkVal.size());
    WriteBytes(rkVal.data(), rkVal.size());
}

void IOutputStream::WriteWideString(const std::wstring& rkVal)
{
    for (unsigned int iChr = 0; iChr < rkVal.size(); iChr++)
        WriteShort(rkVal[iChr]);

    if ((!rkVal.empty()) && (rkVal.back() != '\0'))
        WriteShort(0);
}

void IOutputStream::WriteWideString(const std::wstring& rkVal, unsigned long Count, bool Terminate)
{
    for (unsigned int iChr = 0; iChr < Count; iChr++)
        WriteShort(rkVal[iChr]);

    if (Terminate && (rkVal[Count-1] != 0))
        WriteShort(0);
}

void IOutputStream::WriteSizedWideString(const std::wstring& rkVal)
{
    WriteLong(rkVal.size());
    WriteBytes(rkVal.data(), rkVal.size() * 2);
}

void IOutputStream::WriteToBoundary(unsigned long Boundary, unsigned char Fill)
{
    long Num = Boundary - (Tell() % Boundary);
    if (Num == Boundary) return;
    for (int iByte = 0; iByte < Num; iByte++)
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
