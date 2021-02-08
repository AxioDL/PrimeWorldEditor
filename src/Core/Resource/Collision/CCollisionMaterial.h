#ifndef CCOLLISIONMATERIAL
#define CCOLLISIONMATERIAL

#include <Common/CColor.h>
#include <Common/EGame.h>
#include <Common/Flags.h>

// Game-neutral collision property flags.
// IMPORTANT: This is an incomplete list. Keep in mind that MP2, MP3, and DKCR store
// collision materials via a 64-bit flag where every bit is used, and some flags differ
// between games. Therefore a single enum doesn't have the resolution to represent EVERY
// flag from EVERY game. In the future, the storage medium needs to be changed to a struct,
// OR we need to be okay with excluding certain flags (which we probably are, because a
// lot of them aren't meant to be used by collision geometry, such as the "Player" flag).
enum ECollisionFlag
{
    eCF_Unknown             = 0x00000001,
    eCF_Stone               = 0x00000002,
    eCF_Metal               = 0x00000004,
    eCF_Grass               = 0x00000008,
    eCF_Ice                 = 0x00000010,
    eCF_MetalGrating        = 0x00000020,
    eCF_Phazon              = 0x00000040,
    eCF_Dirt                = 0x00000080,
    eCF_Lava                = 0x00000100,
    eCF_AltMetal            = 0x00000200,
    eCF_Snow                = 0x00000400,
    eCF_Fabric              = 0x00000800,
    eCF_SlowMud             = 0x00001000,
    eCF_Mud                 = 0x00002000,
    eCF_Glass               = 0x00004000,
    eCF_Shield              = 0x00008000,
    eCF_Sand                = 0x00010000,
    eCF_MothSeedOrganics    = 0x00020000,
    eCF_Web                 = 0x00040000,
    eCF_Wood                = 0x00080000,
    eCF_Organic             = 0x00100000,
    eCF_Rubber              = 0x00200000,
    eCF_ShootThru           = 0x00400000,
    eCF_CameraThru          = 0x00800000,
    eCF_ScanThru            = 0x01000000,
    eCF_AiWalkThru          = 0x02000000,
    eCF_FlippedTri          = 0x04000000,
    eCF_Ceiling             = 0x08000000,
    eCF_Wall                = 0x10000000,
    eCF_Floor               = 0x20000000,
    eCF_AiBlock             = 0x40000000,
    eCF_JumpNotAllowed      = 0x80000000
};

class CCollisionMaterial : public TFlags<ECollisionFlag>
{
    friend class CCollisionLoader;
    uint64 mRawFlags = 0;

public:
    ECollisionFlag SurfaceType(EGame Game) const;
    CColor SurfaceColor(EGame Game) const;
    bool IsFloor() const;
    bool IsUnstandable(EGame Game) const;

    uint64 RawFlags() const { return mRawFlags; }
};

#endif // CCOLLISIONMATERIAL

