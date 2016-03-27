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

    TBasicString(u32 Size)
        : mInternalString(Size, 0)
    {
    }

    TBasicString(u32 Size, CharType Fill)
        : mInternalString(Size, Fill)
    {
    }

    TBasicString(const CharType* pkText)
    {
        if (pkText)
            mInternalString = pkText;
    }

    TBasicString(const CharType* pkText, u32 Length)
        : mInternalString(pkText, Length)
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

    inline CharType At(u32 Pos) const
    {
#if _DEBUG
        if (Size() <= Pos)
            throw std::out_of_range("Invalid position passed to TBasicString::At()");
#endif
        return mInternalString.at(Pos);
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
        size_t Pos = mInternalString.find_first_of(pkCharacters);

        if (Pos == _TStdString::npos)
            return -1;
        else
            return (u32) Pos;
    }

    inline u32 LastIndexOf(const CharType* pkCharacters) const
    {
        size_t Pos = mInternalString.find_last_of(pkCharacters);

        if (Pos == _TStdString::npos)
            return -1;
        else
            return (u32) Pos;
    }

    // Modify String
    inline _TString SubString(int StartPos, int Length) const
    {
        return mInternalString.substr(StartPos, Length);
    }

    inline void Insert(u32 Pos, CharType Chr)
    {
#ifdef _DEBUG
        if (Size() < Pos)
            throw std::out_of_range("Invalid pos passed to TBasicString::Insert(CharType)");
#endif
        mInternalString.insert(Pos, 1, Chr);
    }

    inline void Insert(u32 Pos, const CharType* pkStr)
    {
#ifdef _DEBUG
        if (Size() < Pos)
            throw std::out_of_range("Invalid pos passed to TBasicString::Insert(const CharType*)");
#endif
        mInternalString.insert(Pos, pkStr);
    }

    inline void Insert(u32 Pos, const _TString& rkStr)
    {
        Insert(Pos, rkStr.CString());
    }

    inline void Append(CharType Chr)
    {
        mInternalString.append(1, Chr);
    }

    inline void Append(const CharType* pkText)
    {
        mInternalString.append(pkText);
    }

    inline void Append(const _TString& rkStr)
    {
        mInternalString.append(rkStr.CString());
    }

    inline void Prepend(CharType Chr)
    {
        Insert(0, Chr);
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
        _TString Out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            CharType Chr = At(iChar);

            if (Chr >= 'a' && Chr <= 'z')
                Out[iChar] = Chr - 0x20;
            else
                Out[iChar] = Chr;
        }

        return Out;
    }

    _TString ToLower() const
    {
        // todo: doesn't handle accented characters
        _TString Out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            CharType Chr = At(iChar);

            if (Chr >= 'A' && Chr <= 'Z')
                Out[iChar] = Chr + 0x20;
            else
                Out[iChar] = Chr;
        }

        return Out;
    }

    _TString Trimmed() const
    {
        int Start = -1, End = -1;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            if (!IsWhitespace(mInternalString[iChar]))
            {
                Start = iChar;
                break;
            }
        }

        // If start is still -1 then there are no non-whitespace characters in this string. Return early.
        if (Start == -1) return "";

        for (int iChar = Size() - 1; iChar >= 0; iChar--)
        {
            if (!IsWhitespace(mInternalString[iChar]))
            {
                End = iChar + 1;
                break;
            }
        }

        return SubString(Start, End - Start);
    }

    inline _TString Truncate(u32 Amount) const
    {
        return SubString(0, Amount);
    }

    inline _TString ChopFront(u32 Amount) const
    {
        if (Size() <= Amount) return "";
        return SubString(Amount, Size() - Amount);
    }

    inline _TString ChopBack(u32 Amount) const
    {
        if (Size() <= Amount) return "";
        return SubString(0, Size() - Amount);
    }

    u32 Hash32() const
    {
        u32 Hash = 0;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            Hash += At(iChar);
            Hash *= 101;
        }

        return Hash;
    }

    u64 Hash64() const
    {
        u64 Hash = 0;

        for (u32 iChar = 0; iChar < Size(); iChar++)
        {
            Hash += At(iChar);
            Hash *= 101;
        }

        return Hash;
    }

    inline u32 ToInt32(int Base = 16) const
    {
        return std::stoul(mInternalString, nullptr, Base);
    }

    inline u64 ToInt64(int Base = 16) const
    {
        return std::stoull(mInternalString, nullptr, Base);
    }

    void ToInt128(CharType* pOut, int Base = 16) const
    {
        // TODO: only works in base 16
        u64 Part1 = std::stoull(mInternalString.substr(0, 16), nullptr, Base);
        u64 Part2 = std::stoull(mInternalString.substr(16, 16), nullptr, Base);

        if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian)
        {
            IOUtil::SwapBytes(Part1);
            IOUtil::SwapBytes(Part2);
        }

        memcpy(pOut, &Part1, 8);
        memcpy(pOut + 8, &Part2, 8);
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
        _TStringList Out;
        u32 LastSplit = 0;

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
                    if (iChr > LastSplit)
                        Out.push_back(SubString(LastSplit, iChr - LastSplit));

                    LastSplit = iChr + 1;
                    break;
                }
            }
        }

        // Add final string
        if (LastSplit != Length())
            Out.push_back(SubString(LastSplit, Length() - LastSplit));

        return Out;
    }

    void EnsureEndsWith(CharType Chr)
    {
        if (Back() != Chr)
            Append(Chr);
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

    bool StartsWith(const _TString& rkStr) const
    {
        if (Size() < rkStr.Size())
            return false;

        return (SubString(0, rkStr.Size()) == rkStr);
    }

    bool EndsWith(const _TString& rkStr) const
    {
        if (Size() < rkStr.Size())
            return false;

        return (SubString(Size() - rkStr.Size(), rkStr.Size()) == rkStr);
    }

    bool Contains(_TString Str, bool CaseSensitive = true) const
    {
        if (Size() < Str.Size()) return false;

        _TString CheckStr(CaseSensitive ? *this : ToUpper());
        if (CaseSensitive) Str = Str.ToUpper();

        u32 LatestPossibleStart = Size() - Str.Size();
        u32 Match = 0;

        for (u32 iChr = 0; iChr < Size() && iChr < Str.Size(); iChr++)
        {
            // If the current character matches, increment match
            if (CheckStr.At(iChr) == Str.At(Match))
                Match++;

            // Otherwise...
            else
            {
                // We need to also compare this character to the first
                // character of the string (unless we just did that)
                if (Match > 0)
                    iChr--;

                Match = 0;

                if (iChr > LatestPossibleStart)
                    break;
            }

            // If we've matched the entire string, then we can return true
            if (Match == Str.Size()) return true;
        }

        return false;
    }

    bool IsHexString(bool RequirePrefix = false, u32 Width = -1) const
    {
        _TString Str(*this);
        bool HasPrefix = Str.StartsWith("0x");

        // If we're required to match the prefix and prefix is missing, return false
        if (RequirePrefix && !HasPrefix)
            return false;

        if (Width == -1)
        {
            // If the string has the 0x prefix, remove it
            if (HasPrefix)
                Str = Str.ChopFront(2);

            // If we have a variable width then assign the width value to the string size
            Width = Str.Size();
        }

        // If the string starts with the prefix and the length matches the string, remove the prefix
        else if ((Str.Size() == Width + 2) && (HasPrefix))
            Str = Str.ChopFront(2);

        // By this point, the string size and the width should match. If they don't, return false.
        if (Str.Size() != Width) return false;

        // Now we can finally check the actual string and make sure all the characters are valid hex characters.
        for (u32 iChr = 0; iChr < Width; iChr++)
        {
            char Chr = Str[iChr];
            if (!((Chr >= '0') && (Chr <= '9')) &&
                !((Chr >= 'a') && (Chr <= 'f')) &&
                !((Chr >= 'A') && (Chr <= 'F')))
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
        size_t EndPath = mInternalString.find_last_of("\\/");
        return SubString(0, EndPath + 1);
    }

    _TString GetFileName(bool WithExtension = true) const
    {
        size_t EndPath = mInternalString.find_last_of("\\/") + 1;

        if (WithExtension)
        {
            return SubString(EndPath, Size() - EndPath);
        }

        else
        {
            size_t EndName = mInternalString.find_last_of(".");
            return SubString(EndPath, EndName - EndPath);
        }
    }

    _TString GetFileExtension() const
    {
        size_t EndName = mInternalString.find_last_of(".");
        return SubString(EndName + 1, Size() - EndName);
    }

    _TString GetFilePathWithoutExtension() const
    {
        size_t EndName = mInternalString.find_last_of(".");
        return SubString(0, EndName);
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

    inline CharType& operator[](int Pos)
    {
        return mInternalString[Pos];
    }

    inline const CharType& operator[](int Pos) const
    {
        return mInternalString[Pos];
    }

    inline const CharType* operator*() const
    {
        return CString();
    }

    _TString operator+(const CharType* pkOther) const
    {
        u32 Len = CStringLength(pkOther);

        _TString Out(Len + Size());
        memcpy(&Out[0], mInternalString.data(), Size() * sizeof(CharType));
        memcpy(&Out[Size()], pkOther, Len * sizeof(CharType));
        return Out;
    }

    inline _TString operator+(const _TString& rkOther) const
    {
        return (*this + rkOther.CString());
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
        u32 Len = CStringLength(pkLeft);

        _TString Out(Len + rkRight.Size());
        memcpy(&Out[0], pkLeft, Len * sizeof(CharType));
        memcpy(&Out[Len], rkRight.CString(), rkRight.Size() * sizeof(CharType));
        return Out;
    }

    inline friend _TString operator+(const _TStdString& rkLeft, const _TString& rkRight)
    {
        _TString Out(rkLeft.size() + rkRight.Size());
        memcpy(&Out[0], rkLeft.data(), rkLeft.size() * sizeof(CharType));
        memcpy(&Out[rkLeft.size()], rkRight.Data(), rkRight.Size() * sizeof(CharType));
        return Out;
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
    static TBasicString<CharType> FromInt32(s32 Value, int Width = 0, int Base = 16)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::setbase(Base) << std::setw(Width) << std::setfill('0') << Value;
        return sstream.str();
    }

    static TBasicString<CharType> FromInt64(s64 Value, int Width = 0, int Base = 16)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::setbase(Base) << std::setw(Width) << std::setfill('0') << Value;
        return sstream.str();
    }

    static TBasicString<CharType> FromFloat(float Value, int MinDecimals = 1)
    {
        TString Out = std::to_string(Value);
        int NumZeroes = Out.Size() - (Out.IndexOf(".") + 1);

        while (Out.Back() == '0' && NumZeroes > MinDecimals)
        {
            Out = Out.ChopBack(1);
            NumZeroes--;
        }

        return Out;
    }

    static TBasicString<CharType> HexString(unsigned char Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        return HexString((unsigned long) Num, Width, AddPrefix, Uppercase);
    }

    static TBasicString<CharType> HexString(unsigned short Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        return HexString((unsigned long) Num, Width, AddPrefix, Uppercase);
    }

    static TBasicString<CharType> HexString(unsigned long Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        std::basic_stringstream<CharType> sstream;
        sstream << std::hex << std::setw(Width) << std::setfill('0') << Num;

        _TString str = sstream.str();
        if (Uppercase) str = str.ToUpper();
        if (AddPrefix) str.Prepend("0x");
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
