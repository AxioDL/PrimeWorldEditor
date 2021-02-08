#ifndef CUNSUPPORTEDPARTICLELOADER_H
#define CUNSUPPORTEDPARTICLELOADER_H

#include "Core/Resource/CDependencyGroup.h"
#include <memory>

// This class is responsible for loading particle formats that aren't yet fully supported.
// Used for finding dependencies. Split from CUnsupportedFormatLoader for being too big.
class CUnsupportedParticleLoader
{
    std::unique_ptr<CDependencyGroup> mpGroup;
    CUnsupportedParticleLoader() = default;

    // Format-Specific Parameter Loading
    bool ParseParticleParameter(IInputStream& rPART);
    bool ParseElectricParameter(IInputStream& rELSC);
    bool ParseSortedParameter(IInputStream& rSRSC);
    bool ParseSpawnParameter(IInputStream& rSPSC);
    bool ParseSwooshParameter(IInputStream& rSWHC);
    bool ParseDecalParameter(IInputStream& rDPSC);
    bool ParseWeaponParameter(IInputStream& rWPSC);
    bool ParseCollisionResponseParameter(IInputStream& rCRSC);
    bool ParseBurstFireParameter(IInputStream& rBFRC);
    bool ParseUserEvaluatorParameter(IInputStream& rUSRC);
    bool ParseTransformParameter(IInputStream& rXFSC);

    // Function Loading
    void ParseBool(IInputStream& rFile);
    void ParseBoolFunction(IInputStream& rFile);
    void ParseBitfieldFunction(IInputStream& rFile);
    void ParseIntFunction(IInputStream& rFile);
    void ParseFloatFunction(IInputStream& rFile);
    void ParseVectorFunction(IInputStream& rFile);
    void ParseModVectorFunction(IInputStream& rFile);
    void ParseColorFunction(IInputStream& rFile);
    void ParseRotationFunction(IInputStream& rFile);
    void ParseUVFunction(IInputStream& rFile);
    void ParseEmitterFunction(IInputStream& rFile);
    void ParseSoundFunction(IInputStream& rFile);
    void ParseAssetFunction(IInputStream& rFile);
    void ParseSpawnSystemKeyframeData(IInputStream& rFile);
    void ParseKeyframeEmitterData(IInputStream& rFile, const CFourCC& rkFunc, uint32 ElemSize);

public:
    static std::unique_ptr<CDependencyGroup> LoadParticle(IInputStream& rPART, CResourceEntry *pEntry);
};

#endif // CUNSUPPORTEDPARTICLELOADER_H
