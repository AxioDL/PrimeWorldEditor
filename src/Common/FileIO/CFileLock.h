#ifndef CFILELOCK_H
#define CFILELOCK_H

#include <cstdio>
#include "Common/TString.h"

// Maintain a file handle to prevent other processes from accessing the file.
class CFileLock
{
    FILE* mpFile;

public:
    CFileLock()
        : mpFile(nullptr)
    {}

    ~CFileLock()
    {
        Release();
    }

    void Lock(const TString& rkPath)
    {
        Release();
        TWideString WidePath = rkPath.ToUTF16();
        mpFile = _wfopen(*WidePath, L"a+");
    }

    void Release()
    {
        if (mpFile)
            fclose(mpFile);
    }
};

#endif // CFILELOCK_H
