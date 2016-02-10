#ifndef TSTRING_H
#define TSTRING_H

#include "types.h"
#include <FileIO/IOUtil.h>

#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <iomanip>

/* This is a string class which is essentially a wrapper around std::basic_string.
 * The reason for this is because there are a lot of string functions I use very
 * frequently that std::string is missing and this is more convenient than creating
 * all these functions externally. I've chosen to remove access to the default
 * std::basic_string functions and replace them with a custom API for consistency.
 *
 * Most of the file contains an implementation for a template base class, TBasicString.
 * Afterwards we define the following subclasses/typedefs:
 *
 * - TBasicString<char> - TString
 * - TBasicString<wchar_t> - TWideString
 * - std::list<TString> - TStringList
 * - std::list<TWideString> - TWideStringList
 *
 * TString and TWideString have functions for converting between each other. For these
 * functions, TString is expected to be encoded in UTF-8 and TWideString is expected to
 * be encoded in UTF-16.
 */

// ************ TBasicString ************
template<class CharType>
class TBasicString
{
    typedef TBasicString<CharType> _TString;
    typedef std::basic_string<CharType> _TStdString;
    typedef std::list<_TString> _TStringList;

protected:
    _TStdString mInternalString;

public:
    // Constructors
    TBasicString()
        : mInternalString()
    {
    }

    TBasicString(u32 size)
        : mInternalString(size, 0)
    {
    }

    TBasicString(u32 size, CharType fill)
        : mInternalString(size, fill)
    {
    }

    TBasicString(const CharType* pkText)
    {
        if (pkText)
            mInternalString = pkText;
    }

    TBasicString(const CharType* pkText, u32 length)
        : mInternalString(pkText, length)
    {
    }

    TBasicString(const _TStdString& rkText)
        : mInternalString(rkText)
    {
    }

    // Data Accessors
    inline const CharType* CString() const
    {
        return mInternalString.c_str();
    }

    inline const CharType* Data() const
    {
        return mInternalString.data();
    }

    inline CharType At(u32 pos) const
    {
#if _DEBUG
        if (Size() <= pos)
            throw std::out_of_range("Invalid position passed to TBasicString::At()");
#endif
        return mInternalString.at(pos);
    }

    inline CharType Front() const
    {
        return (Size() > 0 ? mInternalString[0] : 0);
    }

    inline CharType Back() const
    {
        return (Size() > 0 ? mInternalString[Size() - 1] : 0);
    }

    inline u32 Size() const
    {
        return mInternalString.size();
    }

    inline u32 Length() const
    {
        return Size();
    }

    inline u32 IndexOf(const CharType* pkCharacters) const
    {
        size_t pos = mInternalString.find_first_of(pkCharacters);

        if (pos == _TStdString::npos)
            return -1;
        else
            return (u32) pos;
    }

    inline u32 LastIndexOf(const CharType* pkCharacters) const
    {
        size_t pos = mInternalString.find_last_of(pkCharacters);

        if (pos == _TStdString::npos)
            return -1;
        else
            return (u32) pos;
    }

    // Modify String
    inline _TString SubString(int startPos, int length) const
    {
        return mInternalString.substr(startPos, length);
    }

    inline void Insert(u32 pos, CharType c)
    {
#ifdef _DEBUG
        if (Size() < pos)
            throw std::out_of_range("Invalid pos passed to TBasicString::Insert(CharType)");
#endif
        mInternalString.insert(pos, 1, c);
    }

    inline void Insert(u32 pos, const CharType* pkStr)
    {
#ifdef _DEBUG
        if (Size() < pos)
            throw std::out_of_range("Invalid pos passed to TBasicString::Insert(const CharType*)");
#endif
        mInternalString.insert(pos, pkStr);
    }

    inline void Insert(u32 pos, const _TString& rkStr)
    {
        Insert(pos, rkStr.CString());
    }

    inline void Append(CharType c)
    {
        mInternalString.append(1, c);
    }

    inline void Append(const CharType* pkText)
    {
        mInternalString.append(pkText);
    }

    inline void Append(const _TString& rkStr)
    {
        mInternalString.append(rkStr.CString());
    }

    inline void Prepend(CharType c)
    {
        Insert(0, c);
    }

    inline void Prepend(const CharType* pkText)
    {
        Insert(0, pkText);
    }

    inline void Prepend(const _TString& rkStr)
    {
        Insert(0, rkStr);
    }

    _TString ToUpper() const
    {
        // todo: doesn't handle accented characters
        _TString out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            CharType c = At(iChar);

            if (c >= 'a' && c <= 'z')
                out[iChar] = c - 0x20;
            else
                out[iChar] = c;
        }

        return out;
    }

    _TString ToLower() const
    {
        // todo: doesn't handle accented characters
        _TString out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            CharType c = At(iChar);

            if (c >= 'A' && c <= 'Z')
                out[iChar] = c + 0x20;
            else
                out[iChar] = c;
        }

        return out;
    }

    _TString Trimmed() const
    {
        int start = -1, end = -1;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            if (!IsWhitespace(mInternalString[iChar]))
            {
                start = iChar;
                break;
            }
        }

        // If start is still -1 then there are no non-whitespace characters in this string. Return early.
        if (start == -1) return "";

        for (int iChar = Size() - 1; iChar >= 0; iChar--)
        {
            if (!IsWhitespace(mInternalString[iChar]))
            {
                end = iChar + 1;
                break;
            }
        }

        return SubString(start, end - start);
    }

    inline _TString Truncate(u32 amount) const
    {
        return SubString(0, amount);
    }

    inline _TString ChopFront(u32 amount) const
    {
        if (Size() <= amount) return "";
        return SubString(amount, Size() - amount);
    }

    inline _TString ChopBack(u32 amount) const
    {
        if (Size() <= amount) return "";
        return SubString(0, Size() - amount);
    }

    u32 Hash32() const
    {
        u32 hash = 0;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            hash += At(iChar);
            hash *= 101;
        }

        return hash;
    }

    u64 Hash64() const
    {
        u64 hash = 0;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            hash += At(iChar);
            hash *= 101;
        }

        return hash;
    }

    inline u32 ToInt32(int base = 16) const
    {
        return std::stoul(mInternalString, nullptr, base);
    }

    inline u64 ToInt64(int base = 16) const
    {
        return std::stoull(mInternalString, nullptr, base);
    }

    void ToInt128(CharType* pOut, int base = 16) const
    {
        // TODO: only works in base 16
        u64 part1 = std::stoull(mInternalString.substr(0, 16), nullptr, base);
        u64 part2 = std::stoull(mInternalString.substr(16, 16), nullptr, base);

        if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian)
        {
            IOUtil::SwapBytes(part1);
            IOUtil::SwapBytes(part2);
        }

        memcpy(pOut, &part1, 8);
        memcpy(pOut + 8, &part2, 8);
    }

    inline float ToFloat() const
    {
        return std::stof(mInternalString, nullptr);
    }

    inline _TStdString ToStdString() const
    {
        return mInternalString;
    }

    _TStringList Split(const CharType* pkTokens) const
    {
        _TStringList out;
        u32 lastSplit = 0;

        // Iterate over all characters in the input string
        for (u32 iChr = 0; iChr < Length(); iChr++)
        {
            // Check whether this character is one of the user-provided tokens
            for (u32 iTok = 0; true; iTok++)
            {
                if (!pkTokens[iTok]) break;

                if (mInternalString[iChr] == pkTokens[iTok])
                {
                    // Token found - split string
                    if (iChr > lastSplit)
                        out.push_back(SubString(lastSplit, iChr - lastSplit));

                    lastSplit = iChr + 1;
                    break;
                }
            }
        }

        // Add final string
        if (lastSplit != Length())
            out.push_back(SubString(lastSplit, Length() - lastSplit));

        return out;
    }

    void EnsureEndsWith(CharType chr)
    {
        if (Back() != chr)
            Append(chr);
    }

    void EnsureEndsWith(const CharType* pkText)
    {
        if (!EndsWith(pkText))
            Append(pkText);
    }

    // Check String
    bool IsEmpty() const
    {
        return (Size() == 0);
    }

    bool StartsWith(const _TString& str) const
    {
        if (Size() < str.Size())
            return false;

        return (SubString(0, str.Size()) == str);
    }

    bool EndsWith(const _TString& str) const
    {
        if (Size() < str.Size())
            return false;

        return (SubString(Size() - str.Size(), str.Size()) == str);
    }

    bool Contains(_TString str, bool caseSensitive = true) const
    {
        if (Size() < str.Size()) return false;

        _TString checkStr(caseSensitive ? *this : ToUpper());
        if (caseSensitive) str = str.ToUpper();

        u32 latestPossibleStart = Size() - str.Size();
        u32 match = 0;

        for (u32 iChr = 0; iChr < Size() && iChr < str.Size(); iChr++)
        {
            // If the current character matches, increment match
            if (checkStr.At(iChr) == str.At(match))
                match++;

            // Otherwise...
            else
            {
                // We need to also compare this character to the first
                // character of the string (unless we just did that)
                if (match > 0)
                    iChr--;

                match = 0;

                if (iChr > latestPossibleStart)
                    break;
            }

            // If we've matched the entire string, then we can return true
            if (match == str.Size()) return true;
        }

        return false;
    }

    bool IsHexString(bool requirePrefix = false, u32 width = -1) const
    {
        _TString str(*this);
        bool hasPrefix = str.StartsWith("0x");

        // If we're required to match the prefix and prefix is missing, return false
        if (requirePrefix && !hasPrefix)
            return false;

        if (width == -1)
        {
            // If the string has the 0x prefix, remove it
            if (hasPrefix)
                str = str.ChopFront(2);

            // If we have a variable width then assign the width value to the string size
            width = str.Size();
        }

        // If the string starts with the prefix and the length matches the string, remove the prefix
        else if ((str.Size() == width + 2) && (hasPrefix))
            str = str.ChopFront(2);

        // By this point, the string size and the width should match. If they don't, return false.
        if (str.Size() != width) return false;

        // Now we can finally check the actual string and make sure all the characters are valid hex characters.
        for (u32 c = 0; c < width; c++)
        {
            char chr = str[c];
            if (!((chr >= '0') && (chr <= '9')) &&
                !((chr >= 'a') && (chr <= 'f')) &&
                !((chr >= 'A') && (chr <= 'F')))
                return false;
        }

        return true;
    }

    inline bool CaseInsensitiveCompare(const _TString& rkOther) const
    {
        return (ToUpper() == rkOther.ToUpper());
    }

    // Get Filename Components
    _TString GetFileDirectory() const
    {
        size_t endPath = mInternalString.find_last_of("\\/");
        return SubString(0, endPath + 1);
    }

    _TString GetFileName(bool withExtension = true) const
    {
        size_t endPath = mInternalString.find_last_of("\\/") + 1;

        if (withExtension)
        {
            return SubString(endPath, Size() - endPath);
        }

        else
        {
            size_t endName = mInternalString.find_last_of(".");
            return SubString(endPath, endName - endPath);
        }
    }

    _TString GetFileExtension() const
    {
        size_t endName = mInternalString.find_last_of(".");
        return SubString(endName + 1, Size() - endName);
    }

    _TString GetFilePathWithoutExtension() const
    {
        size_t endName = mInternalString.find_last_of(".");
        return SubString(0, endName);
    }

    // Operators
    inline _TString& operator=(const CharType* pkText)
    {
        mInternalString = pkText;
        return *this;
    }

    inline _TString& operator=(const _TString& rkText)
    {
        mInternalString = rkText.mInternalString;
        return *this;
    }

    inline CharType& operator[](int pos)
    {
        return mInternalString[pos];
    }

    inline const CharType& operator[](int pos) const
    {
        return mInternalString[pos];
    }

    inline const CharType* operator*() const
    {
        return CString();
    }

    _TString operator+(const CharType* pkOther) const
    {
        u32 len = CStringLength(pkOther);

        _TString out(len + Size());
        memcpy(&out[0], mInternalString.data(), Size() * sizeof(CharType));
        memcpy(&out[Size()], pkOther, len * sizeof(CharType));
        return out;
    }

    inline _TString operator+(const _TString& other) const
    {
        return (*this + other.CString());
    }

    inline void operator+=(const CharType* pkOther)
    {
        *this = *this + pkOther;
    }

    inline void operator+=(const _TString& rkOther)
    {
        *this = *this + rkOther;
    }

    inline friend _TString operator+(const CharType* pkLeft, const _TString& rkRight)
    {
        u32 len = CStringLength(pkLeft);

        _TString out(len + rkRight.Size());
        memcpy(&out[0], pkLeft, len * sizeof(CharType));
        memcpy(&out[len], rkRight.CString(), rkRight.Size() * sizeof(CharType));
        return out;
    }

    inline friend _TString operator+(const _TStdString& rkLeft, const _TString& rkRight)
    {
        _TString out(rkLeft.size() + rkRight.Size());
        memcpy(&out[0], rkLeft.data(), rkLeft.size() * sizeof(CharType));
        memcpy(&out[rkLeft.size()], rkRight.Data(), rkRight.Size() * sizeof(CharType));
        return out;
    }

    inline bool operator==(const CharType *pkText) const
    {
        return CompareCStrings(pkText, CString());
    }

    inline bool operator==(const _TString& rkOther) const
    {
        return (mInternalString == rkOther.mInternalString);
    }

    inline friend bool operator==(const CharType *pkText, const _TString& rkString)
    {
        return (rkString == pkText);
    }

    inline friend bool operator==(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB == rkStringA);
    }

    inline bool operator!=(const CharType *pkText) const
    {
        return (!(*this == pkText));
    }

    inline bool operator!=(const _TString& rkOther) const
    {
        return (!(*this == rkOther));
    }

    inline friend bool operator!=(const CharType *pkText, const _TString& rkString)
    {
        return (rkString != pkText);
    }

    inline friend bool operator!=(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB != rkStringA);
    }

    inline bool operator<(const CharType* pkText) const
    {
        return (mInternalString < pkText);
    }

    inline bool operator<(const _TString& rkOther) const
    {
        return (mInternalString < rkOther.mInternalString);
    }

    inline friend bool operator<(const CharType* pkText, const _TString& rkString)
    {
        return (rkString > pkText);
    }

    inline friend bool operator<(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB > rkStringA);
    }

    inline bool operator<=(const CharType* pkText) const
    {
        return (mInternalString <= pkText);
    }

    inline bool operator<=(const _TString& rkOther) const
    {
        return (mInternalString <= rkOther.mInternalString);
    }

    inline friend bool operator<=(const CharType* pkText, const _TString& rkString)
    {
        return (rkString >= pkText);
    }

    inline friend bool operator<=(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB >= rkStringA);
    }

    inline bool operator>(const CharType* pkText) const
    {
        return (mInternalString > pkText);
    }

    inline bool operator>(const _TString& rkOther) const
    {
        return (mInternalString > rkOther.mInternalString);
    }

    inline friend bool operator>(const CharType* pkText, const _TString& rkString)
    {
        return (rkString < pkText);
    }

    inline friend bool operator>(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB < rkStringA);
    }

    inline bool operator>=(const CharType* pkText) const
    {
        return (mInternalString >= pkText);
    }

    inline bool operator>=(const _TString& rkOther) const
    {
        return (mInternalString >= rkOther.mInternalString);
    }

    inline friend bool operator>=(const CharType* pkText, const _TString& rkString)
    {
        return (rkString <= pkText);
    }

    inline friend bool operator>=(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB <= rkStringA);
    }

    inline friend std::ostream& operator<<(std::ostream& rStream, const _TString& rkString)
    {
        rStream << rkString.mInternalString;
        return rStream;
    }

    inline friend std::istream& operator>>(std::istream& rStream, const _TString& rkString)
    {
        rStream >> rkString.mInternalString;
        return rStream;
    }

    // Static
    static TBasicString<CharType> FromInt32(s32 value, int width = 0, int base = 16)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::setbase(base) << std::setw(width) << std::setfill('0') << value;
        return sstream.str();
    }

    static TBasicString<CharType> FromInt64(s64 value, int width = 0, int base = 16)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::setbase(base) << std::setw(width) << std::setfill('0') << value;
        return sstream.str();
    }

    static TBasicString<CharType> FromFloat(float value, int MinDecimals = 1)
    {
        TString Out = std::to_string(value);
        int NumZeroes = Out.Size() - (Out.IndexOf(".") + 1);

        while (Out.Back() == '0' && NumZeroes > MinDecimals)
        {
            Out = Out.ChopBack(1);
            NumZeroes--;
        }

        return Out;
    }

    static TBasicString<CharType> HexString(unsigned char num, bool addPrefix = true, bool uppercase = false, int width = 0)
    {
        return HexString((unsigned long) num, addPrefix, uppercase, width);
    }

    static TBasicString<CharType> HexString(unsigned short num, bool addPrefix = true, bool uppercase = false, int width = 0)
    {
        return HexString((unsigned long) num, addPrefix, uppercase, width);
    }

    static TBasicString<CharType> HexString(unsigned long  num, bool addPrefix = true, bool uppercase = false, int width = 0)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::hex << std::setw(width) << std::setfill('0') << num;

        _TString str = sstream.str();
        if (uppercase) str = str.ToUpper();
        if (addPrefix) str.Prepend("0x");
        return str;
    }

    static bool CompareCStrings(const CharType* pkA, const CharType* pkB)
    {
        // Replacement for strcmp so we can compare any CharType
        while (true)
        {
            if (*pkA != *pkB) return false;
            if ((*pkA == 0) || (*pkB == 0)) return true;
            pkA++;
            pkB++;
        }
    }

    static u32 CStringLength(const CharType* pkStr)
    {
        // Replacement for strlen so we can measure any CharType
        u32 out = 0;

        while (true)
        {
            if (*pkStr == 0) return out;
            out++;
            pkStr++;
        }
    }

    static bool IsWhitespace(CharType c)
    {
        return ( (c == '\t') ||
                 (c == '\n') ||
                 (c == '\v') ||
                 (c == '\f') ||
                 (c == '\r') ||
                 (c == ' ') );
    }
};

// ************ TString ************
class TString : public TBasicString<char>
{
public:
    TString()                                   : TBasicString<char>() {}
    TString(size_t size)                        : TBasicString<char>(size) {}
    TString(size_t size, char fill)             : TBasicString<char>(size, fill) {}
    TString(const char* pkText)                 : TBasicString<char>(pkText) {}
    TString(const char* pkText, u32 length)     : TBasicString<char>(pkText, length) {}
    TString(const std::string& rkText)          : TBasicString<char>(rkText) {}
    TString(const TBasicString<char>& rkStr)    : TBasicString<char>(rkStr) {}
    TString(const wchar_t* pkText);
    TString(const std::wstring& rkText);
    TString(const class TWideString& rkText);

    class TWideString ToUTF16() const;
};

// ************ TWideString ************
class TWideString : public TBasicString<wchar_t>
{
public:
    TWideString()                                   : TBasicString<wchar_t>() {}
    TWideString(u32 size)                           : TBasicString<wchar_t>(size) {}
    TWideString(u32 size, wchar_t fill)             : TBasicString<wchar_t>(size, fill) {}
    TWideString(const wchar_t* pkText)              : TBasicString<wchar_t>(pkText) {}
    TWideString(const wchar_t* pkText, u32 length)  : TBasicString<wchar_t>(pkText, length) {}
    TWideString(const std::wstring& rkText)         : TBasicString<wchar_t>(rkText) {}
    TWideString(const TBasicString<wchar_t>& rkStr) : TBasicString<wchar_t>(rkStr) {}
    TWideString(const char* pkText);
    TWideString(const std::string& rkText);
    TWideString(const TString& rkText);

    class TString ToUTF8() const;
};

// ************ Typedefs ************
typedef std::list<TBasicString<char>>       TStringList;
typedef std::list<TBasicString<wchar_t>>    TWideStringList;

#endif // TSTRING_H
