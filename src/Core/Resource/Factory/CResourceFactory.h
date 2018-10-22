#ifndef CRESOURCEFACTORY
#define CRESOURCEFACTORY

#include "CAnimationLoader.h"
#include "CAnimEventLoader.h"
#include "CAnimSetLoader.h"
#include "CAreaLoader.h"
#include "CAudioGroupLoader.h"
#include "CCollisionLoader.h"
#include "CDependencyGroupLoader.h"
#include "CFontLoader.h"
#include "CMaterialLoader.h"
#include "CModelLoader.h"
#include "CPoiToWorldLoader.h"
#include "CScanLoader.h"
#include "CScriptLoader.h"
#include "CSkeletonLoader.h"
#include "CSkinLoader.h"
#include "CStringLoader.h"
#include "CTextureDecoder.h"
#include "CUnsupportedFormatLoader.h"
#include "CUnsupportedParticleLoader.h"
#include "CWorldLoader.h"

#include "Core/Resource/Resources.h"

// Static helper class to allow spawning resources based on an EResType
class CResourceFactory
{
    CResourceFactory() {}

public:
    static CResource* SpawnResource(CResourceEntry *pEntry)
    {
        switch (pEntry->ResourceType())
        {
        case eAnimation:            return new CAnimation(pEntry);
        case eAnimEventData:        return new CAnimEventData(pEntry);
        case eAnimSet:              return new CAnimSet(pEntry);
        case eArea:                 return new CGameArea(pEntry);
        case eAudioMacro:           return new CAudioMacro(pEntry);
        case eAudioGroup:           return new CAudioGroup(pEntry);
        case eAudioLookupTable:     return new CAudioLookupTable(pEntry);
        case eCharacter:            return new CAnimSet(pEntry);
        case eDynamicCollision:     return new CCollisionMeshGroup(pEntry);
        case eDependencyGroup:      return new CDependencyGroup(pEntry);
        case eFont:                 return new CFont(pEntry);
        case eMapArea:              return new CMapArea(pEntry);
        case eModel:                return new CModel(pEntry);
        case eScan:                 return new CScan(pEntry);
        case eSkeleton:             return new CSkeleton(pEntry);
        case eSkin:                 return new CSkin(pEntry);
        case eSourceAnimData:       return new CSourceAnimData(pEntry);
        case eStaticGeometryMap:    return new CPoiToWorld(pEntry);
        case eStringList:           return new CStringList(pEntry);
        case eStringTable:          return new CStringTable(pEntry);
        case eTexture:              return new CTexture(pEntry);
        case eWorld:                return new CWorld(pEntry);
        default:                    return nullptr; // should it return a CResource instead?
        }
    }

    static CResource* LoadCookedResource(CResourceEntry *pEntry, IInputStream& rInput)
    {
        // Warning: It is the caller's responsibility to check if the required resource is already in memory before calling this function.
        if (!rInput.IsValid()) return nullptr;
        CResource *pRes = nullptr;

        switch (pEntry->ResourceType())
        {
        case eAnimation:            pRes = CAnimationLoader::LoadANIM(rInput, pEntry);          break;
        case eAnimEventData:        pRes = CAnimEventLoader::LoadEVNT(rInput, pEntry);          break;
        case eAnimSet:              pRes = CAnimSetLoader::LoadANCS(rInput, pEntry);            break;
        case eArea:                 pRes = CAreaLoader::LoadMREA(rInput, pEntry);               break;
        case eAudioMacro:           pRes = CUnsupportedFormatLoader::LoadCAUD(rInput, pEntry);  break;
        case eAudioGroup:           pRes = CAudioGroupLoader::LoadAGSC(rInput, pEntry);         break;
        case eAudioLookupTable:     pRes = CAudioGroupLoader::LoadATBL(rInput, pEntry);         break;
        case eBinaryData:           pRes = CUnsupportedFormatLoader::LoadDUMB(rInput, pEntry);  break;
        case eCharacter:            pRes = CAnimSetLoader::LoadCHAR(rInput, pEntry);            break;
        case eDependencyGroup:      pRes = CDependencyGroupLoader::LoadDGRP(rInput, pEntry);    break;
        case eDynamicCollision:     pRes = CCollisionLoader::LoadDCLN(rInput, pEntry);          break;
        case eFont:                 pRes = CFontLoader::LoadFONT(rInput, pEntry);               break;
        case eGuiFrame:             pRes = CUnsupportedFormatLoader::LoadFRME(rInput, pEntry);  break;
        case eHintSystem:           pRes = CUnsupportedFormatLoader::LoadHINT(rInput, pEntry);  break;
        case eMapArea:              pRes = CUnsupportedFormatLoader::LoadMAPA(rInput, pEntry);  break;
        case eMapWorld:             pRes = CUnsupportedFormatLoader::LoadMAPW(rInput, pEntry);  break;
        case eMapUniverse:          pRes = CUnsupportedFormatLoader::LoadMAPU(rInput, pEntry);  break;
        case eMidi:                 pRes = CUnsupportedFormatLoader::LoadCSNG(rInput, pEntry);  break;
        case eModel:                pRes = CModelLoader::LoadCMDL(rInput, pEntry);              break;
        case eRuleSet:              pRes = CUnsupportedFormatLoader::LoadRULE(rInput, pEntry);  break;
        case eScan:                 pRes = CScanLoader::LoadSCAN(rInput, pEntry);               break;
        case eSkeleton:             pRes = CSkeletonLoader::LoadCINF(rInput, pEntry);           break;
        case eSkin:                 pRes = CSkinLoader::LoadCSKR(rInput, pEntry);               break;
        case eSourceAnimData:       pRes = CAnimSetLoader::LoadSAND(rInput, pEntry);            break;
        case eStateMachine2:        pRes = CUnsupportedFormatLoader::LoadFSM2(rInput, pEntry);  break;
        case eStaticGeometryMap:    pRes = CPoiToWorldLoader::LoadEGMC(rInput, pEntry);         break;
        case eStringList:           pRes = CAudioGroupLoader::LoadSTLC(rInput, pEntry);         break;
        case eStringTable:          pRes = CStringLoader::LoadSTRG(rInput, pEntry);             break;
        case eTexture:              pRes = CTextureDecoder::LoadTXTR(rInput, pEntry);           break;
        case eWorld:                pRes = CWorldLoader::LoadMLVL(rInput, pEntry);              break;

        case eStateMachine:
            // AFSM currently unsupported
            if (pEntry->Game() <= EGame::Echoes)
                pRes = new CDependencyGroup(pEntry);
            else if (pEntry->Game() <= EGame::Corruption)
                pRes = CUnsupportedFormatLoader::LoadFSM2(rInput, pEntry);
            else if (pEntry->Game() == EGame::DKCReturns)
                pRes = CUnsupportedFormatLoader::LoadFSMC(rInput, pEntry);
            break;

        case eBurstFireData:
        case eParticle:
        case eParticleElectric:
        case eParticleSorted:
        case eParticleSpawn:
        case eParticleSwoosh:
        case eParticleDecal:
        case eParticleWeapon:
        case eParticleCollisionResponse:
        case eParticleTransform:
        case eUserEvaluatorData:
            pRes = CUnsupportedParticleLoader::LoadParticle(rInput, pEntry);
            break;

        default:
            pRes = new CResource(pEntry);
            break;
        }

        return pRes;
    }
};

#endif // CRESOURCEFACTORY

