#include "TString.h"
#include <FileIO/IOUtil.h>

// ************ TString ************
TString::TString(const wchar_t* pkText)
{
    *this = TWideString(pkText).ToUTF8();
}

TString::TString(const std::wstring& rkText)
{
    *this = TWideString(rkText).ToUTF8();
}

TString::TString(const TWideString& rkText)
{
    *this = rkText.ToUTF8();
}

TWideString TString::ToUTF16() const
{
    TWideString out;
    const char *pkCStr = CString();

    while (pkCStr[0])
    {
        // Step 1: decode UTF-8 code point
        wchar_t CodePoint;

        // One byte
        if ((pkCStr[0] & 0x80) == 0)
        {
            CodePoint = pkCStr[0] & 0x7FFFFFFF;
            pkCStr++;
        }

        // Two bytes
        else if ((pkCStr[0] & 0xE0) == 0xC0)
        {
            CodePoint = (((pkCStr[0] & 0x1F) << 6) |
                          (pkCStr[1] & 0x3F));
            pkCStr += 2;
        }

        // Three bytes
        else if ((pkCStr[0] & 0xF0) == 0xE0)
        {
            CodePoint = (((pkCStr[0] & 0xF)  << 12) |
                         ((pkCStr[1] & 0x3F) <<  6) |
                          (pkCStr[2] & 0x3F));
            pkCStr += 3;
        }

        // Four bytes
        else if ((pkCStr[0] & 0xF8) == 0xF0)
        {
            CodePoint = (((pkCStr[0] & 0x7)  << 18) |
                         ((pkCStr[1] & 0x3F) << 12) |
                         ((pkCStr[2] & 0x3F) <<  6) |
                          (pkCStr[3] & 0x3F));
            pkCStr += 4;
        }

        // Five bytes
        else if ((pkCStr[0] & 0xFC) == 0xF8)
        {
            CodePoint = (((pkCStr[0] & 0x3)  << 24) |
                         ((pkCStr[1] & 0x3F) << 18) |
                         ((pkCStr[2] & 0x3F) << 12) |
                         ((pkCStr[3] & 0x3F) <<  6) |
                          (pkCStr[4] & 0x3F));
            pkCStr += 5;
        }

        // Six bytes
        else if ((pkCStr[0] & 0xFE) == 0xFC)
        {
            CodePoint = (((pkCStr[0] & 0x1)  << 30) |
                         ((pkCStr[1] & 0x3F) << 24) |
                         ((pkCStr[2] & 0x3F) << 18) |
                         ((pkCStr[3] & 0x3F) << 12) |
                         ((pkCStr[4] & 0x3F) <<  6) |
                          (pkCStr[5] & 0x3F));
            pkCStr += 6;
        }

        // Step 2: Append to output string
        if ( ((CodePoint >= 0)      && (CodePoint <= 0xD7FF)) ||
             ((CodePoint >= 0xE000) && (CodePoint <= 0xFFFF)) )
            out.Append((wchar_t) (CodePoint & 0xFFFF));
    }

    return out;
}

// ************ TWideString ************
TWideString::TWideString(const char* pkText)
{
    *this = TString(pkText).ToUTF16();
}

TWideString::TWideString(const std::string& rkText)
{
    *this = TString(rkText).ToUTF16();
}

TWideString::TWideString(const TString& rkText)
{
    *this = rkText.ToUTF16();
}

TString TWideString::ToUTF8() const
{
    return "UTF16 to UTF8 currently unsupported";
}
