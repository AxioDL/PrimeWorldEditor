#include "IOUtil.h"

namespace IOUtil
{
    EEndianness FindSystemEndianness()
    {
        // Memory layout for a 32-bit value of 1:
        // 0x01000000 - Little Endian
        // 0x00000001 - Big Endian
        long EndianTest = 1;
        if (*(char*)&EndianTest == 1) return LittleEndian;
        else return BigEndian;
    }
    const EEndianness SystemEndianness = FindSystemEndianness();

    void SwapBytes(short& Val)
    {
        Val = (((Val & 0x00FF) << 8) |
               ((Val & 0xFF00) >> 8));
    }

    void SwapBytes(unsigned short& Val)
    {
        Val = (((Val & 0x00FF) << 8) |
               ((Val & 0xFF00) >> 8));
    }

    void SwapBytes(long& Val)
    {
        Val = (((Val & 0x000000FF) << 24) |
               ((Val & 0x0000FF00) <<  8) |
               ((Val & 0x00FF0000) >>  8) |
               ((Val & 0xFF000000) >> 24));
    }

    void SwapBytes(unsigned long& Val)
    {
        Val = (((Val & 0x000000FF) << 24) |
               ((Val & 0x0000FF00) <<  8) |
               ((Val & 0x00FF0000) >>  8) |
               ((Val & 0xFF000000) >> 24));
    }

    void SwapBytes(long long& Val)
    {
        Val = (((Val & 0x00000000000000FF) << 56) |
               ((Val & 0x000000000000FF00) << 40) |
               ((Val & 0x0000000000FF0000) << 24) |
               ((Val & 0x00000000FF000000) <<  8) |
               ((Val & 0x000000FF00000000) >>  8) |
               ((Val & 0x0000FF0000000000) >> 24) |
               ((Val & 0x00FF000000000000) >> 40) |
               ((Val & 0xFF00000000000000) >> 56));
    }

    void SwapBytes(unsigned long long &Val)
    {
        Val = (((Val & 0x00000000000000FF) << 56) |
               ((Val & 0x000000000000FF00) << 40) |
               ((Val & 0x0000000000FF0000) << 24) |
               ((Val & 0x00000000FF000000) <<  8) |
               ((Val & 0x000000FF00000000) >>  8) |
               ((Val & 0x0000FF0000000000) >> 24) |
               ((Val & 0x00FF000000000000) >> 40) |
               ((Val & 0xFF00000000000000) >> 56));
    }

    void SwapBytes(float& Val)
    {
        long* ptr = (long*) &Val;
        SwapBytes(*ptr);
    }

    void SwapBytes(double& Val)
    {
        long long* ptr = (long long*) &Val;
        SwapBytes(*ptr);
    }
}
