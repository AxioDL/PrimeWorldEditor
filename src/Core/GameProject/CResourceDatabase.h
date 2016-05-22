#ifndef CRESOURCEDATABASE_H
#define CRESOURCEDATABASE_H

#include <Common/CUniqueID.h>
#include <Common/TString.h>
#include <Common/types.h>

class CResourceEntry
{
    CUniqueID ID;
    TString DataPath;

public:
};

class CResourceDatabase
{
    struct SResEntry
    {
        CUniqueID ID;
        TString DataPath;
    };

public:
    CResourceDatabase() {}
    ~CResourceDatabase() {}
};

#endif // CRESOURCEDATABASE_H
