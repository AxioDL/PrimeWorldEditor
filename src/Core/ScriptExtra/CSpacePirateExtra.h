#ifndef CSPACEPIRATEEXTRA_H
#define CSPACEPIRATEEXTRA_H

#include "CScriptExtra.h"
#include "Core/Resource/Script/IProperty.h"

class CSpacePirateExtra : public CScriptExtra
{
    // Render beam troopers with the correct color
    TEnumProperty *mpPowerVuln;
    TEnumProperty *mpWaveVuln;
    TEnumProperty *mpIceVuln;
    TEnumProperty *mpPlasmaVuln;

public:
    explicit CSpacePirateExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent = 0);
    CColor TevColor();
};

#endif // CSPACEPIRATEEXTRA_H
