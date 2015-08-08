#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "StringUtil.h"

#include <FileIO/IOUtil.h> // For SwapBytes

namespace StringUtil
{
    std::string GetFileDirectory(std::string path)
    {
        size_t endpath = path.find_last_of("\\/");
        return path.substr(0, endpath + 1);
    }

    std::string GetFileName(std::string path)
    {
        size_t endpath = path.find_last_of("\\/") + 1;
        size_t endname = path.find_last_of(".");
        return path.substr(endpath, endname - endpath);
    }

    std::string GetFileNameWithExtension(std::string path)
    {
        size_t endpath = path.find_last_of("\\/");
        return path.substr(endpath + 1, path.size() - endpath);
    }

    std::string GetPathWithoutExtension(std::string path)
    {
        size_t endname = path.find_last_of(".");
        return path.substr(0, endname);
    }

    std::string GetExtension(std::string path)
    {
        size_t endname = path.find_last_of(".");
        return path.substr(endname + 1, path.size() - endname);
    }


    // Not convinced stringstream is the best way to do string conversions of asset IDs - don't know of a better way tho
    std::string ResToStr(unsigned long assetID)
    {
        std::stringstream sstream;
        sstream << std::hex << std::setw(8) << std::setfill('0') << assetID << std::dec;
        return sstream.str();
    }

    std::string ResToStr(unsigned long long assetID)
    {
        std::stringstream sstream;
        sstream << std::hex << std::setw(16) << std::setfill('0') << assetID << std::dec;
        return sstream.str();
    }

    std::string ToUpper(std::string str)
    {
        for (unsigned int i = 0; i < str.length(); i++)
        {
            if ((str[i] >= 0x61) && (str[i] <= 0x7A))
                str[i] -= 0x20;
        }

        return str;
    }

    std::string ToLower(std::string str)
    {
        for (unsigned int i = 0; i < str.length(); i++)
        {
            if ((str[i] >= 0x41) && (str[i] <= 0x5A))
                str[i] += 0x20;
        }

        return str;
    }

    std::string ToHexString(unsigned char num, bool addPrefix, int width)
    {
        return ToHexString((unsigned long) num, addPrefix, width);
    }

    std::string ToHexString(unsigned short num, bool addPrefix, int width)
    {
        return ToHexString((unsigned long) num, addPrefix, width);
    }

    std::string ToHexString(unsigned long num, bool addPrefix, int width)
    {
        std::stringstream str;
        if (addPrefix) str << "0x";
        str << std::hex << std::setw(width) << std::setfill('0') << num;
        return str.str();
    }

    long Hash32(std::string str)
    {
        unsigned long hash = 0;

        for (unsigned int c = 0; c < str.size(); c++) {
            hash += str[c];
            hash *= 101;
        }

        return hash;
    }

    long long Hash64(std::string str)
    {
        unsigned long long hash = 0;

        for (unsigned int c = 0; c < str.size(); c++) {
            hash += str[c];
            hash *= 101;
        }

        return hash;
    }

    long StrToRes32(std::string str) {
        return std::stoul(str, nullptr, 16);
    }

    long long StrToRes64(std::string str) {
        return std::stoull(str, nullptr, 16);
    }

    void StrToRes128(std::string str, char *out) {
        long long Part1 = std::stoull(str.substr(0, 16), nullptr, 16);
        long long Part2 = std::stoull(str.substr(16, 16), nullptr, 16);

        if (IOUtil::SystemEndianness == IOUtil::LittleEndian)
        {
            IOUtil::SwapBytes(Part1);
            IOUtil::SwapBytes(Part2);
        }

        memcpy(out, &Part1, 8);
        memcpy(out + 8, &Part2, 8);
    }

    long GetResID32(std::string str)
    {
        long resID;
        if (IsHexString(str, false, 8))
            resID = StrToRes32(str);
        else
            resID = Hash32(GetFileName(str));
        return resID;
    }

    bool IsHexString(std::string str, bool requirePrefix, long width)
    {
        str = GetFileName(str);

        if (requirePrefix && (str.substr(0, 2) != "0x"))
            return false;

        if ((width == -1) && (str.substr(0, 2) == "0x"))
            str = str.substr(2, str.size() - 2);

        if (width == -1)
            width = str.size();

        if ((str.size() == width + 2) && (str.substr(0, 2) == "0x"))
            str = str.substr(2, width);

        if (str.size() != width) return false;

        for (int c = 0; c < width; c++)
        {
            char chr = str[c];
            if (!((chr >= '0') && (chr <= '9')) &&
                !((chr >= 'a') && (chr <= 'f')) &&
                !((chr >= 'A') && (chr <= 'F')))
                return false;
        }
        return true;
    }

    std::string AppendSlash(std::string str)
    {
        char a = str.back();
        char b = str[str.length() - 1];

        if (a == 0)
        {
            if ((b != '/') && (b != '\\'))
            {
                str.back() = '/';
                str.push_back(0);
            }
        }

        else if ((a != '/') && (b != '\\'))
            str.push_back('/');

        return str;
    }

    std::wstring UTF8to16(std::string str)
    {
        const char *cstr = str.c_str();
        std::vector<int> CodePoints;

        // Step 1: decode UTF-8 code points
        while (cstr[0])
        {
            int CodePoint;

            // One byte
            if ((cstr[0] & 0x80) == 0)
            {
                CodePoint = cstr[0] & 0x7FFFFFFF;
                cstr++;
            }

            // Two bytes
            else if ((cstr[0] & 0xE0) == 0xC0)
            {
                CodePoint = (((cstr[0] & 0x1F) << 6) |
                              (cstr[1] & 0x3F));
                cstr += 2;
            }

            // Three bytes
            else if ((cstr[0] & 0xF0) == 0xE0)
            {
                CodePoint = (((cstr[0] & 0xF)  << 12) |
                             ((cstr[1] & 0x3F) <<  6) |
                              (cstr[2] & 0x3F));
                cstr += 3;
            }

            // Four bytes
            else if ((cstr[0] & 0xF8) == 0xF0)
            {
                CodePoint = (((cstr[0] & 0x7)  << 18) |
                             ((cstr[1] & 0x3F) << 12) |
                             ((cstr[2] & 0x3F) <<  6) |
                              (cstr[3] & 0x3F));
                cstr += 4;
            }

            // Five bytes
            else if ((cstr[0] & 0xFC) == 0xF8)
            {
                CodePoint = (((cstr[0] & 0x3)  << 24) |
                             ((cstr[1] & 0x3F) << 18) |
                             ((cstr[2] & 0x3F) << 12) |
                             ((cstr[3] & 0x3F) <<  6) |
                              (cstr[4] & 0x3F));
                cstr += 5;
            }

            // Six bytes
            else if ((cstr[0] & 0xFE) == 0xFC)
            {
                CodePoint = (((cstr[0] & 0x1)  << 30) |
                             ((cstr[1] & 0x3F) << 24) |
                             ((cstr[2] & 0x3F) << 18) |
                             ((cstr[3] & 0x3F) << 12) |
                             ((cstr[4] & 0x3F) <<  6) |
                              (cstr[5] & 0x3F));
                cstr += 6;
            }

            CodePoints.push_back(CodePoint);
        }

        // Step 2: encode as UTF-16
        std::wstring out;
        out.reserve(CodePoints.size());

        for (int c = 0; c < CodePoints.size(); c++)
        {
            // todo: support all code points
            if (((CodePoints[c] >= 0) && (CodePoints[c] <= 0xD7FF)) ||
                ((CodePoints[c] >= 0xE000) && (CodePoints[c] <= 0xFFFF)))
            {
                out.push_back(CodePoints[c] & 0xFFFF);
            }
        }

        return out;
    }

    CStringList Tokenize(const std::string& str, const char *pTokens)
    {
        CStringList out;
        int lastSplit = 0;

        // Iterate over all characters in the input string
        for (int iChr = 0; iChr < str.length(); iChr++)
        {
            // Check whether this character is one of the user-provided tokens
            for (int iTok = 0; true; iTok++)
            {
                if (!pTokens[iTok]) break;

                if (str[iChr] == pTokens[iTok])
                {
                    // Token found - split string
                    if (iChr > lastSplit)
                        out.push_back(str.substr(lastSplit, iChr - lastSplit));

                    lastSplit = iChr + 1;
                    break;
                }
            }
        }

        // Add final string
        if (lastSplit != str.length())
            out.push_back(str.substr(lastSplit, str.length() - lastSplit));

        return out;
    }
}
