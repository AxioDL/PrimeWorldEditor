#ifndef CSPACEPIRATEEXTRA_H
#define CSPACEPIRATEEXTRA_H

#include "CScriptExtra.h"
#include "Core/Resource/Script/IProperty.h"

class CSpacePirateExtra : public CScriptExtra
{
    // Render beam troopers with the correct color
    TLongProperty *mpPowerVuln;
    TLongProperty *mpWaveVuln;
    TLongProperty *mpIceVuln;
    TLongProperty *mpPlasmaVuln;

public:
    explicit CSpacePirateExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent = 0);
    CColor TevColor();
};

#endif // CSPACEPIRATEEXTRA_H
