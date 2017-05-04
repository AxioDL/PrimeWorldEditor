#include "IOUtil.h"

namespace IOUtil
{
    EEndianness FindSystemEndianness()
    {
        // Memory layout for a 32-bit value of 1:
        // 0x01000000 - Little Endian
        // 0x00000001 - Big Endian
        long EndianTest = 1;
        if (*(char*)&EndianTest == 1) return eLittleEndian;
        else return eBigEndian;
    }
    const EEndianness kSystemEndianness = FindSystemEndianness();

    void SwapBytes(short& rVal)
    {
        rVal = (((rVal & 0x00FF) << 8) |
               ((rVal & 0xFF00) >> 8));
    }

    void SwapBytes(unsigned short& rVal)
    {
        rVal = (((rVal & 0x00FF) << 8) |
               ((rVal & 0xFF00) >> 8));
    }

    void SwapBytes(long& rVal)
    {
        rVal = (((rVal & 0x000000FF) << 24) |
               ((rVal & 0x0000FF00) <<  8) |
               ((rVal & 0x00FF0000) >>  8) |
               ((rVal & 0xFF000000) >> 24));
    }

    void SwapBytes(unsigned long& rVal)
    {
        rVal = (((rVal & 0x000000FF) << 24) |
               ((rVal & 0x0000FF00) <<  8) |
               ((rVal & 0x00FF0000) >>  8) |
               ((rVal & 0xFF000000) >> 24));
    }

    void SwapBytes(long long& rVal)
    {
        rVal = (((rVal & 0x00000000000000FF) << 56) |
               ((rVal & 0x000000000000FF00) << 40) |
               ((rVal & 0x0000000000FF0000) << 24) |
               ((rVal & 0x00000000FF000000) <<  8) |
               ((rVal & 0x000000FF00000000) >>  8) |
               ((rVal & 0x0000FF0000000000) >> 24) |
               ((rVal & 0x00FF000000000000) >> 40) |
               ((rVal & 0xFF00000000000000) >> 56));
    }

    void SwapBytes(unsigned long long& rVal)
    {
        rVal = (((rVal & 0x00000000000000FF) << 56) |
               ((rVal & 0x000000000000FF00) << 40) |
               ((rVal & 0x0000000000FF0000) << 24) |
               ((rVal & 0x00000000FF000000) <<  8) |
               ((rVal & 0x000000FF00000000) >>  8) |
               ((rVal & 0x0000FF0000000000) >> 24) |
               ((rVal & 0x00FF000000000000) >> 40) |
               ((rVal & 0xFF00000000000000) >> 56));
    }

    void SwapBytes(float& rVal)
    {
        long* pPtr = (long*) &rVal;
        SwapBytes(*pPtr);
    }

    void SwapBytes(double& rVal)
    {
        long long* pPtr = (long long*) &rVal;
        SwapBytes(*pPtr);
    }
}
