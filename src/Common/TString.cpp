#include "TString.h"
#include "Hash/CCRC32.h"
#include "Hash/CFNV1A.h"
#include "FileIO/IOUtil.h"
#include <codecvt>
#include <locale>

// ************ TString ************
u32 TString::Hash32() const
{
    CCRC32 Hash;
    Hash.Hash(**this);
    return Hash.Digest();
}

u64 TString::Hash64() const
{
    // todo: replace with MD5
    CFNV1A Hash(CFNV1A::e64Bit);
    Hash.HashString(*this);
    return Hash.GetHash64();
}

TWideString TString::ToUTF16() const
{
    TWideString Out;
    Out.Reserve(Size());

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

        // Invalid?
        else
        {
            CodePoint = pkCStr[0];
            pkCStr++;
        }

        // Step 2: Append to output string
        if ( ((CodePoint >= 0)      && (CodePoint <= 0xD7FF)) ||
             ((CodePoint >= 0xE000) && (CodePoint <= 0xFFFF)) )
            Out.Append((wchar_t) (CodePoint & 0xFFFF));
    }

    Out.Shrink();
    return Out;
}

// ************ TWideString ************
u32 TWideString::Hash32() const
{
    CFNV1A Hash(CFNV1A::e32Bit);
    Hash.HashData(Data(), Size() * sizeof(wchar_t));
    return Hash.GetHash32();
}

u64 TWideString::Hash64() const
{
    CFNV1A Hash(CFNV1A::e64Bit);
    Hash.HashData(Data(), Size() * sizeof(wchar_t));
    return Hash.GetHash64();
}

TString TWideString::ToUTF8() const
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Convert;
    return TString( Convert.to_bytes(ToStdString()) );
}
