#ifndef TSTRING_H
#define TSTRING_H

#include "types.h"
#include "Common/FileIO/IOUtil.h"

#include <cstdarg>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>
#include <vector>

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

// Helper macros for creating string literals of the correct char type. Internal use only! Invalid outside of this header!
#define LITERAL(Text) (typeid(CharType) == typeid(char) ? (const CharType*) ##Text : (const CharType*) L##Text)
#define CHAR_LITERAL(Text) (CharType) Text

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
#ifdef _DEBUG
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

    inline u32 IndexOf(CharType Character, u32 Offset) const
    {
        size_t Pos = mInternalString.find_first_of(Character, Offset);
        return (Pos == _TStdString::npos ? -1 : (u32) Pos);
    }

    inline u32 IndexOf(CharType Character) const
    {
        return IndexOf(Character, 0);
    }

    inline u32 IndexOf(const CharType* pkCharacters, u32 Offset) const
    {
        size_t Pos = mInternalString.find_first_of(pkCharacters, Offset);
        return (Pos == _TStdString::npos ? -1 : (u32) Pos);
    }

    inline u32 IndexOf(const CharType* pkCharacters) const
    {
        return IndexOf(pkCharacters, 0);
    }

    inline u32 LastIndexOf(CharType Character) const
    {
        size_t Pos = mInternalString.find_last_of(Character);
        return (Pos == _TStdString::npos ? -1 : (u32) Pos);
    }

    inline u32 LastIndexOf(const CharType* pkCharacters) const
    {
        size_t Pos = mInternalString.find_last_of(pkCharacters);

        if (Pos == _TStdString::npos)
            return -1;
        else
            return (u32) Pos;
    }

    u32 IndexOfPhrase(const _TString& rkStr, u32 Offset, bool CaseSensitive = true) const
    {
        if (Size() < rkStr.Size()) return -1;

        // Now loop from the offset provided by the user.
        u32 Pos = Offset;
        u32 LatestPossibleStart = Size() - rkStr.Size();
        u32 MatchStart = -1;
        u32 Matched = 0;

        while (Pos < Size())
        {
            // If this character matches, increment Matched!
            bool Match = CaseSensitive ? (At(Pos) == rkStr[Matched]) : (CharToUpper(At(Pos)) == CharToUpper(rkStr[Matched]));

            if (Match)
            {
                Matched++;

                if (MatchStart == -1)
                    MatchStart = Pos;

                // If we matched the entire string, we can return.
                if (Matched == rkStr.Size())
                    return MatchStart;
            }

            else
            {
                // If we didn't match, clear our existing match check.
                if (Matched > 0)
                {
                    Pos = MatchStart;
                    Matched = 0;
                    MatchStart = -1;
                }

                // Check if we're too far in to find another match.
                if (Pos > LatestPossibleStart) break;
            }

            Pos++;
        }

        return -1;
    }

    inline u32 IndexOfPhrase(const _TString& rkStr, bool CaseSensitive = true) const
    {
        return IndexOfPhrase(rkStr, 0, CaseSensitive);
    }

    // Modify String
    inline _TString SubString(int StartPos, int Length) const
    {
        return mInternalString.substr(StartPos, Length);
    }

    inline void Reserve(u32 Amount)
    {
        mInternalString.reserve(Amount);
    }

    inline void Shrink()
    {
        mInternalString.shrink_to_fit();
    }

    inline void Insert(u32 Pos, CharType Chr)
    {
#ifdef _DEBUG
        if (Size() < Pos)
            throw std::out_of_range("Invalid position passed to TBasicString::Insert()");
#endif
        mInternalString.insert(Pos, 1, Chr);
    }

    inline void Insert(u32 Pos, const CharType* pkStr)
    {
#ifdef _DEBUG
        if (Size() < Pos)
            throw std::out_of_range("Invalid position passed to TBasicString::Insert()");
#endif
        mInternalString.insert(Pos, pkStr);
    }

    inline void Insert(u32 Pos, const _TString& rkStr)
    {
        Insert(Pos, rkStr.CString());
    }

    inline void Remove(u32 Pos, u32 Len)
    {
#ifdef _DEBUG
        if (Size() <= Pos)
            throw std::out_of_range("Invalid position passed to TBasicString::Remove()");
#endif
        mInternalString.erase(Pos, Len);
    }

    inline void Remove(const CharType* pkStr, bool CaseSensitive = false)
    {
        u32 InStrLen = CStringLength(pkStr);

        for (u32 Idx = IndexOfPhrase(pkStr, CaseSensitive); Idx != -1; Idx = IndexOfPhrase(pkStr, Idx, CaseSensitive))
            Remove(Idx, InStrLen);
    }

    inline void Remove(CharType Chr)
    {
        for (u32 Idx = IndexOf(Chr); Idx != -1; Idx = IndexOf(Chr, Idx))
            Remove(Idx, 1);
    }

    inline void RemoveWhitespace()
    {
        for (u32 Idx = 0; Idx < Size(); Idx++)
        {
            if (IsWhitespace(At(Idx)))
            {
                Remove(Idx, 1);
                Idx--;
            }
        }
    }

    inline void Replace(const CharType* pkStr, const CharType *pkReplacement, bool CaseSensitive = false)
    {
        u32 Offset = 0;
        u32 InStrLen = CStringLength(pkStr);
        u32 ReplaceStrLen = CStringLength(pkReplacement);

        for (u32 Idx = IndexOfPhrase(pkStr, CaseSensitive); Idx != -1; Idx = IndexOfPhrase(pkStr, Offset, CaseSensitive))
        {
            Remove(Idx, InStrLen);
            Insert(Idx, pkReplacement);
            Offset = Idx + ReplaceStrLen;
        }
    }

    inline void Replace(const _TString& rkStr, const _TString& rkReplacement, bool CaseSensitive)
    {
        Replace(rkStr.CString(), rkReplacement.CString(), CaseSensitive);
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
        _TString Out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
            Out[iChar] = CharToUpper( At(iChar) );

        return Out;
    }

    _TString ToLower() const
    {
        // todo: doesn't handle accented characters
        _TString Out(Size());

        for (u32 iChar = 0; iChar < Size(); iChar++)
            Out[iChar] = CharToLower( At(iChar) );

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
        if (Start == -1) return _TString();

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
        if (Size() <= Amount) return _TString();
        return SubString(Amount, Size() - Amount);
    }

    inline _TString ChopBack(u32 Amount) const
    {
        if (Size() <= Amount) return _TString();
        return SubString(0, Size() - Amount);
    }

    inline u32 ToInt32(int Base = 16) const
    {
        try {
            return std::stoul(mInternalString, nullptr, Base);
        }
        catch(...) {
            return 0;
        }
    }

    inline u64 ToInt64(int Base = 16) const
    {
        try {
            return std::stoull(mInternalString, nullptr, Base);
        }
        catch(...) {
            return 0;
        }
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

    _TStringList Split(const CharType* pkTokens, bool KeepEmptyParts = false) const
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
                    if (iChr > LastSplit || KeepEmptyParts)
                        Out.push_back(SubString(LastSplit, iChr - LastSplit));

                    LastSplit = iChr + 1;
                    break;
                }
            }
        }

        // Add final string
        if (LastSplit != Length() || KeepEmptyParts)
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

    bool StartsWith(CharType Chr, bool CaseSensitive = true) const
    {
        if (IsEmpty())
            return false;

        return CaseSensitive ? Front() == Chr : CharToUpper(Front()) == CharToUpper(Chr);
    }

    bool StartsWith(const _TString& rkStr, bool CaseSensitive = true) const
    {
        if (Size() < rkStr.Size())
            return false;

        _TString SubStr = SubString(0, rkStr.Size());
        return CaseSensitive ? SubStr == rkStr : SubStr.CaseInsensitiveCompare(rkStr);
    }

    bool EndsWith(CharType Chr, bool CaseSensitive = true) const
    {
        if (IsEmpty())
            return false;

        return CaseSensitive ? Back() == Chr : CharToUpper(Back()) == CharToUpper(Chr);
    }

    bool EndsWith(const _TString& rkStr, bool CaseSensitive = true) const
    {
        if (Size() < rkStr.Size())
            return false;

        _TString SubStr = SubString(Size() - rkStr.Size(), rkStr.Size());
        return CaseSensitive ? SubStr == rkStr : SubStr.CaseInsensitiveCompare(rkStr);
    }

    bool Contains(_TString Str, bool CaseSensitive = true) const
    {
        return (IndexOfPhrase(Str, CaseSensitive) != -1);
    }

    bool Contains(CharType Chr) const
    {
        return IndexOf(Chr) != -1;
    }

    bool IsHexString(bool RequirePrefix = false, u32 Width = -1) const
    {
        _TString Str(*this);
        bool HasPrefix = Str.StartsWith(LITERAL("0x"));

        // If we're required to match the prefix and prefix is missing, return false
        if (RequirePrefix && !HasPrefix)
            return false;

        if (Width == -1)
        {
            // If the string has the 0x prefix, remove it
            if (HasPrefix)
                Str = Str.ChopFront(2);

            // If the string is empty other than the prefix, then this is not a valid hex string
            if (Str.IsEmpty())
                return false;

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
            CharType Chr = Str[iChr];
            if (!((Chr >= CHAR_LITERAL('0')) && (Chr <= CHAR_LITERAL('9'))) &&
                !((Chr >= CHAR_LITERAL('a')) && (Chr <= CHAR_LITERAL('f'))) &&
                !((Chr >= CHAR_LITERAL('A')) && (Chr <= CHAR_LITERAL('F'))))
                return false;
        }

        return true;
    }

    inline bool CaseInsensitiveCompare(const _TString& rkOther) const
    {
        if (Size() != rkOther.Size())
            return false;

        for (u32 iChr = 0; iChr < Size(); iChr++)
            if (CharToUpper(At(iChr)) != CharToUpper(rkOther[iChr]))
                return false;

        return true;
    }

    // Get Filename Components
    _TString GetFileDirectory() const
    {
        size_t EndPath = mInternalString.find_last_of(LITERAL("\\/"));
        return EndPath == _TStdString::npos ? LITERAL("") : SubString(0, EndPath + 1);
    }

    _TString GetFileName(bool WithExtension = true) const
    {
        size_t EndPath = mInternalString.find_last_of(LITERAL("\\/")) + 1;

        if (WithExtension)
        {
            return SubString(EndPath, Size() - EndPath);
        }

        else
        {
            size_t EndName = mInternalString.find_last_of(LITERAL("."));
            return SubString(EndPath, EndName - EndPath);
        }
    }

    _TString GetFileExtension() const
    {
        size_t EndName = mInternalString.find_last_of(LITERAL("."));
        return EndName == _TStdString::npos ? LITERAL("") : SubString(EndName + 1, Size() - EndName);
    }

    _TString GetFilePathWithoutExtension() const
    {
        size_t EndName = mInternalString.find_last_of(LITERAL("."));
        return EndName == _TStdString::npos ? *this : SubString(0, EndName);
    }

    _TString GetParentDirectoryPath(const _TString& rkParentDirName, bool CaseSensitive = true)
    {
        int IdxA = 0;
        int IdxB = IndexOf(LITERAL("\\/"));
        if (IdxB == -1) return _TString();

        while (IdxB != -1)
        {
            _TString DirName = SubString(IdxA, IdxB - IdxA);

            if (CaseSensitive ? (DirName == rkParentDirName) : (DirName.CaseInsensitiveCompare(rkParentDirName)))
                return Truncate(IdxB + 1);

            IdxA = IdxB + 1;
            IdxB = IndexOf(LITERAL("\\/"), IdxA);
        }

        return _TString();
    }

    // Operators
    inline _TString& operator=(CharType Char)
    {
        mInternalString = Char;
        return *this;
    }

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

    _TString operator+(CharType Other) const
    {
        _TString Out(Size() + 1);
        memcpy(&Out[0], mInternalString.data(), Size() * sizeof(CharType));
        memcpy(&Out[Size()], &Other, sizeof(CharType));
        return Out;
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

    inline void operator+=(CharType Other)
    {
        *this = *this + Other;
    }

    inline void operator+=(const CharType* pkOther)
    {
        *this = *this + pkOther;
    }

    inline void operator+=(const _TString& rkOther)
    {
        *this = *this + rkOther;
    }

    inline friend _TString operator+(CharType Left, const _TString& rkRight)
    {
        _TString Out(rkRight.Size() + 1);
        memcpy(&Out[0], &Left, sizeof(CharType));
        memcpy(&Out[sizeof(CharType)], rkRight.CString(), rkRight.Size() * sizeof(CharType));
        return Out;
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

    inline bool operator==(CharType Other) const
    {
        return Size() == 1 && At(0) == Other;
    }

    inline bool operator==(const CharType *pkText) const
    {
        return CompareCStrings(pkText, CString());
    }

    inline bool operator==(const _TString& rkOther) const
    {
        return (mInternalString == rkOther.mInternalString);
    }

    inline friend bool operator==(CharType Other, const _TString& rkString)
    {
        return (rkString == Other);
    }

    inline friend bool operator==(const CharType *pkText, const _TString& rkString)
    {
        return (rkString == pkText);
    }

    inline friend bool operator==(const _TStdString& rkStringA, const _TString& rkStringB)
    {
        return (rkStringB == rkStringA);
    }

    inline bool operator!=(CharType Other) const
    {
        return (!(*this == Other));
    }

    inline bool operator!=(const CharType *pkText) const
    {
        return (!(*this == pkText));
    }

    inline bool operator!=(const _TString& rkOther) const
    {
        return (!(*this == rkOther));
    }

    inline friend bool operator!=(CharType Other, const _TString& rkString)
    {
        return (rkString != Other);
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
    static _TString Format(const CharType *pkFmt, ...)
    {
        // Probably better to rewrite this at some point for better error handling + avoiding all the C-style syntax
        const int kBufferSize = 4096;
        CharType StringBuffer[kBufferSize];

        std::va_list Args;
        va_start(Args, pkFmt);

        if (typeid(CharType) == typeid(char))
            vsprintf_s((char*) StringBuffer, kBufferSize, (char*) pkFmt, Args);
        else
            vswprintf_s((wchar_t*) StringBuffer, kBufferSize, (wchar_t*) pkFmt, Args);

        va_end(Args);
        return _TString(StringBuffer);
    }

    static _TString FromInt32(s32 Value, int Width = 0, int Base = 16)
    {
        std::basic_stringstream<CharType> SStream;
        SStream << std::setbase(Base) << std::setw(Width) << std::setfill(CHAR_LITERAL('0')) << Value;
        return SStream.str();
    }

    static _TString FromInt64(s64 Value, int Width = 0, int Base = 16)
    {
        std::basic_stringstream<CharType> SStream;
        SStream << std::setbase(Base) << std::setw(Width) << std::setfill(CHAR_LITERAL('0')) << Value;
        return SStream.str();
    }

    static _TString FromFloat(float Value, int MinDecimals = 1, bool Scientific = false)
    {
        _TString Out = _TString::Format(Scientific ? "%.8g" : "%f", Value);

        // Make sure we have the right number of decimals
        int DecIdx = Out.IndexOf(CHAR_LITERAL('.'));

        if (DecIdx == -1 && MinDecimals > 0)
        {
            DecIdx = Out.Size();
            Out.Append(CHAR_LITERAL('.'));
        }

        int NumZeroes = (DecIdx == -1 ? 0 : Out.Size() - (DecIdx + 1));

        // Add extra zeroes to meet the minimum decimal count
        if (NumZeroes < MinDecimals)
        {
            for (int iDec = 0; iDec < (MinDecimals - NumZeroes); iDec++)
                Out.Append(CHAR_LITERAL('0'));
        }

        // Remove unnecessary trailing zeroes from the end of the string
        else if (NumZeroes > MinDecimals)
        {
            while (Out.Back() == CHAR_LITERAL('0') && NumZeroes > MinDecimals && NumZeroes > 0)
            {
                Out = Out.ChopBack(1);
                NumZeroes--;
            }

            // Remove decimal point
            if (NumZeroes == 0)
                Out = Out.ChopBack(1);
        }

        return Out;
    }

    static _TString FileSizeString(u64 Size, u32 NumDecimals = 2)
    {
        _TString Out;
        _TString Type;

        if (Size < 100)
        {
            return FromInt64(Size, 0, 10) + LITERAL(" bytes");
        }

        else if (Size < 1000000)
        {
            Out = FromFloat(Size / 1000.f, NumDecimals);
            Type = LITERAL("KB");
        }

        else if (Size < 1000000000)
        {
            Out = FromFloat(Size / 1000000.f, NumDecimals);
            Type = LITERAL("MB");
        }

        else
        {
            Out = FromFloat(Size / 1000000000.f, NumDecimals);
            Type = LITERAL("GB");
        }

        u32 DecCount = Out.Size() - (Out.IndexOf(CHAR_LITERAL('.')) + 1);
        if (DecCount > NumDecimals) Out = Out.ChopBack(DecCount - NumDecimals);
        return Out + Type;
    }

    static _TString HexString(unsigned char Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        return HexString((unsigned long) Num, Width, AddPrefix, Uppercase);
    }

    static _TString HexString(unsigned short Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        return HexString((unsigned long) Num, Width, AddPrefix, Uppercase);
    }

    static _TString HexString(unsigned long Num, int Width = 8, bool AddPrefix = true, bool Uppercase = true)
    {
        std::basic_stringstream<CharType> SStream;
        SStream << std::hex << std::setw(Width) << std::setfill('0') << Num;

        _TString Str = SStream.str();
        if (Uppercase) Str = Str.ToUpper();
        if (AddPrefix) Str.Prepend(LITERAL("0x"));
        return Str;
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
        u32 Out = 0;

        while (true)
        {
            if (*pkStr == 0) return Out;
            Out++;
            pkStr++;
        }
    }

    inline static CharType CharToLower(CharType Chr)
    {
        // todo: doesn't handle accented characters
        return (Chr >= CHAR_LITERAL('A') && Chr <= CHAR_LITERAL('Z')) ? Chr + 0x20 : Chr;
    }

    inline static CharType CharToUpper(CharType Chr)
    {
        // todo: doesn't handle accented characters
        return (Chr >= CHAR_LITERAL('a') && Chr <= CHAR_LITERAL('z')) ? Chr - 0x20 : Chr;
    }

    static bool IsVowel(CharType Chr)
    {
        Chr = CharToUpper(Chr);
        return (Chr == 'A' ||
                Chr == 'E' ||
                Chr == 'I' ||
                Chr == 'O' ||
                Chr == 'U');
    }

    static bool IsWhitespace(CharType Chr)
    {
        return ( (Chr == CHAR_LITERAL('\t')) ||
                 (Chr == CHAR_LITERAL('\n')) ||
                 (Chr == CHAR_LITERAL('\v')) ||
                 (Chr == CHAR_LITERAL('\f')) ||
                 (Chr == CHAR_LITERAL('\r')) ||
                 (Chr == CHAR_LITERAL(' '))  );
    }

    static inline bool IsNumerical(CharType Chr)
    {
        return (Chr >= CHAR_LITERAL('0') && Chr <= CHAR_LITERAL('9'));
    }
};

#undef LITERAL
#undef CHAR_LITERAL

// ************ TString ************
class TString : public TBasicString<char>
{
public:
    TString()                                   : TBasicString<char>() {}
    TString(size_t Size)                        : TBasicString<char>(Size) {}
    TString(size_t Size, char Fill)             : TBasicString<char>(Size, Fill) {}
    TString(const char* pkText)                 : TBasicString<char>(pkText) {}
    TString(const char* pkText, u32 Length)     : TBasicString<char>(pkText, Length) {}
    TString(const std::string& rkText)          : TBasicString<char>(rkText) {}
    TString(const TBasicString<char>& rkStr)    : TBasicString<char>(rkStr) {}

    u32 Hash32() const;
    u64 Hash64() const;
    class TWideString ToUTF16() const;
};

// ************ TWideString ************
class TWideString : public TBasicString<wchar_t>
{
public:
    TWideString()                                   : TBasicString<wchar_t>() {}
    TWideString(u32 Size)                           : TBasicString<wchar_t>(Size) {}
    TWideString(u32 Size, wchar_t Fill)             : TBasicString<wchar_t>(Size, Fill) {}
    TWideString(const wchar_t* pkText)              : TBasicString<wchar_t>(pkText) {}
    TWideString(const wchar_t* pkText, u32 Length)  : TBasicString<wchar_t>(pkText, Length) {}
    TWideString(const std::wstring& rkText)         : TBasicString<wchar_t>(rkText) {}
    TWideString(const TBasicString<wchar_t>& rkStr) : TBasicString<wchar_t>(rkStr) {}

    u32 Hash32() const;
    u64 Hash64() const;
    class TString ToUTF8() const;
};

// ************ Typedefs ************
typedef std::list<TBasicString<char>>       TStringList;
typedef std::list<TBasicString<wchar_t>>    TWideStringList;

#endif // TSTRING_H
