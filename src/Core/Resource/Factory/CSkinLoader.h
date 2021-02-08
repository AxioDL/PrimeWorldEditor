#ifndef CSKINLOADER_H
#define CSKINLOADER_H

#include "Core/Resource/Animation/CSkin.h"
#include "Core/Resource/TResPtr.h"
#include <memory>

class CSkinLoader
{
    CSkinLoader() = default;

public:
    static std::unique_ptr<CSkin> LoadCSKR(IInputStream& rCSKR, CResourceEntry *pEntry);
};

#endif // CSKINLOADER_H
