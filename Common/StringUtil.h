#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <list>
#include <string>
typedef std::list<std::string> CStringList;

namespace StringUtil
{
    std::string GetFileDirectory(std::string path);
    std::string GetFileName(std::string path);
    std::string GetFileNameWithExtension(std::string path);
    std::string GetPathWithoutExtension(std::string path);
    std::string GetExtension(std::string path);
    std::string ResToStr(unsigned long ID);
    std::string ResToStr(unsigned long long ID);
    std::string ToUpper(std::string str);
    std::string ToLower(std::string str);
    std::string ToHexString(unsigned char num, bool addPrefix = true, int width = 0);
    std::string ToHexString(unsigned short num, bool addPrefix = true, int width = 0);
    std::string ToHexString(unsigned long num, bool addPrefix = true, int width = 0);
    long      Hash32(std::string str);
    long long Hash64(std::string str);
    long      StrToRes32(std::string str);
    long long StrToRes64(std::string str);
    void      StrToRes128(std::string str, char *out);
    long      GetResID32(std::string str);
    bool IsHexString(std::string str, bool requirePrefix = false, long width = -1);
    std::string AppendSlash(std::string str);
    CStringList Tokenize(const std::string& str, const char *pTokens);

    std::wstring UTF8to16(std::string str);
}

#endif // STRINGUTIL_H
