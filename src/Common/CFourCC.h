#ifndef CFOURCC_H
#define CFOURCC_H

#include "types.h"
#include "TString.h"
#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CFourCC
{
    char mFourCC[4];
public:
    // Constructors
    CFourCC();
    CFourCC(const char *src);
    CFourCC(const TString& src);
    CFourCC(u32 src);
    CFourCC(IInputStream& src);

    // Functionality
    void Write(IOutputStream& Output);
    u32 ToLong() const;
    TString ToString() const;
    CFourCC ToUpper() const;

    // Operators
    CFourCC& operator=(const char *src);
    CFourCC& operator=(const TString& src);
    CFourCC& operator=(u32 src);
    bool operator==(const CFourCC& other) const;
    bool operator!=(const CFourCC& other) const;
    bool operator>(const CFourCC& other) const;
    bool operator>=(const CFourCC& other) const;
    bool operator<(const CFourCC& other) const;
    bool operator<=(const CFourCC& other) const;
    char operator[](int index);
    const char operator[](int index) const;
};

#endif // CFOURCC_H
