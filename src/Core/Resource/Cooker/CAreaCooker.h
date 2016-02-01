#ifndef CAREACOOKER_H
#define CAREACOOKER_H

#include "CSectionMgrOut.h"
#include "Core/Resource/CGameArea.h"
#include "Core/Resource/EGame.h"
#include <FileIO/FileIO.h>

class CAreaCooker
{
    TResPtr<CGameArea> mpArea;
    EGame mVersion;

    CSectionMgrOut mSectionMgr;
    u32 mSectionSizesOffset;

    u32 mGeometrySecNum;
    u32 mSclySecNum;
    u32 mCollisionSecNum;
    u32 mUnknownSecNum;
    u32 mLightsSecNum;
    u32 mVisiSecNum;
    u32 mPathSecNum;
    u32 mArotSecNum;

    CAreaCooker();
    void DetermineSectionNumbers();
    void WritePrimeHeader(IOutputStream& rOut);
    void WritePrimeSCLY(IOutputStream& rOut);

public:
    static void WriteCookedArea(CGameArea *pArea, IOutputStream& rOut);
    static u32 GetMREAVersion(EGame version);
};

#endif // CAREACOOKER_H
