#ifndef CFOURCC_H
#define CFOURCC_H

#include <FileIO/CInputStream.h>
#include <FileIO/COutputStream.h>
#include <string>

class CFourCC
{
    char FourCC[4];
public:
    CFourCC();
    CFourCC(const char *src);
    CFourCC(const std::string& src);
    CFourCC(long src);
    CFourCC(CInputStream& src);
    void Write(COutputStream& Output);

    CFourCC& operator=(const char *src);
    CFourCC& operator=(const std::string& src);
    CFourCC& operator=(long src);
    bool operator==(const CFourCC& other) const;
    bool operator!=(const CFourCC& other) const;
    bool operator==(const char *other) const;
    bool operator!=(const char *other) const;
    bool operator==(const long other) const;
    bool operator!=(const long other) const;

    long ToLong() const;
    std::string ToString() const;
    CFourCC ToUpper() const;
};

#endif // CFOURCC_H
