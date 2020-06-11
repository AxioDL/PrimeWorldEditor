#ifndef CSKINLOADER_H
#define CSKINLOADER_H

#include "Core/Resource/Animation/CSkin.h"
#include "Core/Resource/TResPtr.h"

class CSkinLoader
{
    CSkinLoader() = default;

public:
    static CSkin* LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry);
};

#endif // CSKINLOADER_H
