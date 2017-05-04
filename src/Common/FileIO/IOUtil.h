#ifndef IOUTIL_H
#define IOUTIL_H

namespace IOUtil
{
    enum EEndianness {
        eLittleEndian,
        eBigEndian
    };
    extern const EEndianness kSystemEndianness;

    void SwapBytes(short& rVal);
    void SwapBytes(unsigned short& rVal);
    void SwapBytes(long& rVal);
    void SwapBytes(unsigned long& rVal);
    void SwapBytes(long long& rVal);
    void SwapBytes(unsigned long long& rVal);
    void SwapBytes(float& rVal);
    void SwapBytes(double& rVal);
}

#endif // IOUTIL_H
