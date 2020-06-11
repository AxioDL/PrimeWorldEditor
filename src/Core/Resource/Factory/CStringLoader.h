#ifndef CSTRINGLOADER_H
#define CSTRINGLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/TResPtr.h"
#include "Core/Resource/StringTable/CStringTable.h"
#include <Common/EGame.h>
#include <memory>

class CStringLoader
{
    CStringTable *mpStringTable = nullptr;
    EGame mVersion{};

    CStringLoader() = default;
    void LoadPrimeDemoSTRG(IInputStream& STRG);
    void LoadPrimeSTRG(IInputStream& STRG);
    void LoadCorruptionSTRG(IInputStream& STRG);
    void LoadNameTable(IInputStream& STRG);

public:
    static std::unique_ptr<CStringTable> LoadSTRG(IInputStream& STRG, CResourceEntry* pEntry);
    static EGame GetFormatVersion(uint Version);
};

#endif // CSTRINGLOADER_H
