#ifndef CFOURCC_H
#define CFOURCC_H

#include "types.h"
#include <FileIO/CInputStream.h>
#include <FileIO/COutputStream.h>
#include <string>

class CFourCC
{
    char mFourCC[4];
public:
    // Constructors
    CFourCC();
    CFourCC(const char *src);
    CFourCC(const std::string& src);
    CFourCC(u32 src);
    CFourCC(CInputStream& src);

    // Functionality
    void Write(COutputStream& Output);
    u32 ToLong() const;
    std::string ToString() const;
    CFourCC ToUpper() const;

    // Operators
    CFourCC& operator=(const char *src);
    CFourCC& operator=(const std::string& src);
    CFourCC& operator=(u32 src);
    bool operator==(const CFourCC& other) const;
    bool operator!=(const CFourCC& other) const;
    bool operator>(const CFourCC& other) const;
    bool operator>=(const CFourCC& other) const;
    bool operator<(const CFourCC& other) const;
    bool operator<=(const CFourCC& other) const;

};

#endif // CFOURCC_H
