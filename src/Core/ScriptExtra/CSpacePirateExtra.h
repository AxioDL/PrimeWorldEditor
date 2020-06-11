#ifndef CSPACEPIRATEEXTRA_H
#define CSPACEPIRATEEXTRA_H

#include "CScriptExtra.h"
#include "Core/Resource/Script/Property/IProperty.h"

enum class EVulnerabilityTypeMP1
{
    DoubleDamage,
    Normal,
    Reflect,
    Immune,
    PassThru,
    DirectDouble,
    DirectNormal,
    DirectImmune
};

class CSpacePirateExtra : public CScriptExtra
{
    // Render beam troopers with the correct color
    TEnumRef<EVulnerabilityTypeMP1> mPowerVulnerability;
    TEnumRef<EVulnerabilityTypeMP1> mWaveVulnerability;
    TEnumRef<EVulnerabilityTypeMP1> mIceVulnerability;
    TEnumRef<EVulnerabilityTypeMP1> mPlasmaVulnerability;

public:
    explicit CSpacePirateExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent = nullptr);
    CColor TevColor() override;
};

#endif // CSPACEPIRATEEXTRA_H
