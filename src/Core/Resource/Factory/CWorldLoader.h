#ifndef CWORLDLOADER_H
#define CWORLDLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CWorld.h"
#include <Common/EGame.h>
#include <Common/FileIO.h>
#include <memory>

class CWorldLoader
{
    TResPtr<CWorld> mpWorld;
    EGame mVersion{};

    CWorldLoader();
    void LoadPrimeMLVL(IInputStream& rMLVL);
    void LoadReturnsMLVL(IInputStream& rMLVL);
    void GenerateEditorData();

public:
    static std::unique_ptr<CWorld> LoadMLVL(IInputStream& rMLVL, CResourceEntry *pEntry);
    static EGame GetFormatVersion(uint32 Version);
};

#endif // CWORLDLOADER_H
