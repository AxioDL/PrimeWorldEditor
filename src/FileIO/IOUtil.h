#ifndef IOUTIL_H
#define IOUTIL_H

namespace IOUtil
{
    enum EEndianness {
        LittleEndian,
        BigEndian
    };
    extern const EEndianness SystemEndianness;

    void SwapBytes(short& Val);
    void SwapBytes(unsigned short& Val);
    void SwapBytes(long& Val);
    void SwapBytes(unsigned long& Val);
    void SwapBytes(long long& Val);
    void SwapBytes(unsigned long long& Val);
    void SwapBytes(float& Val);
    void SwapBytes(double& Val);
}

#endif // IOUTIL_H
