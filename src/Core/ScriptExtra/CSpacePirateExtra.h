#ifndef CSPACEPIRATEEXTRA_H
#define CSPACEPIRATEEXTRA_H

#include "CScriptExtra.h"
#include "Core/Resource/Script/CProperty.h"

class CSpacePirateExtra : public CScriptExtra
{
    // Render beam troopers with the correct color
    CLongProperty *mpPowerVuln;
    CLongProperty *mpWaveVuln;
    CLongProperty *mpIceVuln;
    CLongProperty *mpPlasmaVuln;

public:
    explicit CSpacePirateExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent = 0);
    CColor TevColor();
};

#endif // CSPACEPIRATEEXTRA_H
