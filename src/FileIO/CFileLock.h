#ifndef CFILELOCK_H
#define CFILELOCK_H

#include <cstdio>

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

    void Lock(const char *pkPath)
    {
        Release();
        mpFile = fopen(pkPath, "a+");
    }

    void Release()
    {
        if (mpFile)
            fclose(mpFile);
    }
};

#endif // CFILELOCK_H
