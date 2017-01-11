#include "CCollisionMaterial.h"
#include "EGame.h"
#include <map>

ECollisionFlag CCollisionMaterial::SurfaceType(EGame Game) const
{
    // Arrays determining the type hierarchy for each game.
    // Flags earlier in the list take priority over flags later in the list.
    static const ECollisionFlag skPrimeTypeHierarchy[] = {
        eCF_Organic, eCF_Wood, eCF_Sand, eCF_Shield, eCF_Glass, eCF_Mud, eCF_SlowMud,
        eCF_Snow, eCF_Lava, eCF_Dirt, eCF_Phazon, eCF_MetalGrating, eCF_Ice, eCF_Grass,
        eCF_Metal, eCF_Stone
    };
    static const ECollisionFlag skEchoesTypeHierarchy[] = {
        eCF_Rubber, eCF_Organic, eCF_Wood, eCF_Web, eCF_MothSeedOrganics, eCF_Sand,
        eCF_Shield, eCF_Fabric, eCF_Snow, eCF_Glass, eCF_AltMetal, eCF_Dirt, eCF_Phazon,
        eCF_MetalGrating, eCF_Ice, eCF_Grass, eCF_Metal, eCF_Stone
    };

    // Determine which list we should use.
    const ECollisionFlag* pkFlagArray;
    u32 Num;

    if (Game <= ePrime)
    {
        pkFlagArray = skPrimeTypeHierarchy;
        Num = sizeof(skPrimeTypeHierarchy) / sizeof(ECollisionFlag);
    }
    else
    {
        pkFlagArray = skEchoesTypeHierarchy;
        Num = sizeof(skEchoesTypeHierarchy) / sizeof(ECollisionFlag);
    }

    // Locate type.
    for (u32 iType = 0; iType < Num; iType++)
    {
        if (*this & pkFlagArray[iType])
            return pkFlagArray[iType];
    }
    return eCF_Unknown;
}

// Type-to-color mappings
const std::map<ECollisionFlag, CColor> gkTypeToColor = {
    { eCF_Stone,        CColor::Integral(220, 215, 160) }, // Brownish/greenish color
    { eCF_Metal,        CColor::Integral(110, 110, 110) }, // Dark gray
    { eCF_Grass,        CColor::Integral( 90, 150,  70) }, // Green
    { eCF_Ice,          CColor::Integral(200, 255, 255) }, // Light blue
    { eCF_MetalGrating, CColor::Integral(170, 170, 170) }, // Gray
    { eCF_Phazon,       CColor::Integral(  0, 128, 255) }, // Blue
    { eCF_Dirt,         CColor::Integral(150, 130, 120) }, // Brownish-gray
    { eCF_Lava,         CColor::Integral(200,  30,  30) }, // Red
    { eCF_Snow,         CColor::Integral(230, 255, 255) }, // *Very* light blue
    { eCF_Glass,        CColor::Integral( 20, 255, 190) }, // Greenish blue
    { eCF_Shield,       CColor::Integral(230, 250,  60) }, // Yellow
    { eCF_Sand,         CColor::Integral(230, 200, 170) }, // Light brown
    { eCF_Wood,         CColor::Integral(190, 140, 105) }, // Brown
    { eCF_Organic,      CColor::Integral(130, 130, 250) }  // Purple
};
CColor CCollisionMaterial::SurfaceColor(EGame Game) const
{
    ECollisionFlag SurfType = SurfaceType(Game);
    auto FindColor = gkTypeToColor.find(SurfType);
    return (FindColor == gkTypeToColor.end() ? CColor::skWhite : FindColor->second);
}
