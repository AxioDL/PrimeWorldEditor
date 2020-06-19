#include "CScanLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include "CScriptLoader.h"
#include <Common/Log.h>

std::unique_ptr<CScan> CScanLoader::LoadScanMP1(IInputStream& SCAN, CResourceEntry* pEntry)
{
    // Validate magic
    const uint32 Magic = SCAN.ReadULong();

    if (Magic != 0x0BADBEEF)
    {
        errorf("Invalid magic in SCAN asset: 0x%08X", Magic);
        return nullptr;
    }

    auto ptr = std::make_unique<CScan>(pEntry);

    // The SCAN format in MP2 and later games uses the script loader to load SCAN parameters.
    // The MP1 format is not loaded the same way, as far as I'm aware, and is loaded the same
    // way as a normal file format... however, since we support all games, we need to support
    // the script object method for proper MP2/3 support (including dealing with property names/IDs).
    // So, it's simplest to use the script loader to load the MP1 SCAN format as well... that enables
    // us to just create one class for all SCAN assets that works for every game.
    mpScan = ptr.get();
    CScriptLoader::LoadStructData(SCAN, mpScan->ScanData());

    return ptr;
}

std::unique_ptr<CScan> CScanLoader::LoadScanMP2(IInputStream& SCAN, CResourceEntry* pEntry)
{
    // Validate version
    const uint32 Version = SCAN.ReadULong();

    if (Version != 2)
    {
        errorf("Unrecognized SCAN version: %d", Version);
        return nullptr;
    }

    auto ptr = std::make_unique<CScan>(pEntry);

    // The SCAN format in MP2 embeds a ScannableObjectInfo script object using the same file format as SCLY.
    // As such we use CScriptLoader to load parameters, but since we don't actually want to create a script
    // object, we will skip past the script object/layer header and just load the properties directly.
    SCAN.Skip(0x17);
    mpScan = ptr.get();
    CScriptLoader::LoadStructData(SCAN, mpScan->ScanData());
    return ptr;
}

// ************ STATIC/PUBLIC ************
std::unique_ptr<CScan> CScanLoader::LoadSCAN(IInputStream& SCAN, CResourceEntry *pEntry)
{
    if (!SCAN.IsValid())
        return nullptr;

    // MP1 SCAN format starts with a version number and then follows with a magic.
    // The demo can be 1, 2, or 3, while the final build is 5.
    // The MP2 SCAN format starts with a 'SCAN' magic.
    const uint32 VersionCheck = SCAN.ReadULong();

    // Echoes+
    if (VersionCheck == FOURCC('SCAN'))
    {
        CScanLoader Loader;
        return Loader.LoadScanMP2(SCAN, pEntry);
    }

    // MP1
    if (VersionCheck <= 5)
    {
        if (VersionCheck == 5)
        {
            CScanLoader Loader;
            return Loader.LoadScanMP1(SCAN, pEntry);
        }
        else
        {
            errorf("%s: Unsupported SCAN version: %u", VersionCheck);
            return nullptr;
        }
    }
    else
    {
        errorf("Failed to identify SCAN version: 0x%X", VersionCheck);
        return nullptr;
    }
}
