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

#include "Core/Tweaks/CTweakLoader.h"

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
        case EResourceType::Animation:            return new CAnimation(pEntry);
        case EResourceType::AnimEventData:        return new CAnimEventData(pEntry);
        case EResourceType::AnimSet:              return new CAnimSet(pEntry);
        case EResourceType::Area:                 return new CGameArea(pEntry);
        case EResourceType::AudioMacro:           return new CAudioMacro(pEntry);
        case EResourceType::AudioGroup:           return new CAudioGroup(pEntry);
        case EResourceType::AudioLookupTable:     return new CAudioLookupTable(pEntry);
        case EResourceType::Character:            return new CAnimSet(pEntry);
        case EResourceType::DynamicCollision:     return new CCollisionMeshGroup(pEntry);
        case EResourceType::DependencyGroup:      return new CDependencyGroup(pEntry);
        case EResourceType::Font:                 return new CFont(pEntry);
        case EResourceType::MapArea:              return new CMapArea(pEntry);
        case EResourceType::Model:                return new CModel(pEntry);
        case EResourceType::Scan:                 return new CScan(pEntry);
        case EResourceType::Skeleton:             return new CSkeleton(pEntry);
        case EResourceType::Skin:                 return new CSkin(pEntry);
        case EResourceType::SourceAnimData:       return new CSourceAnimData(pEntry);
        case EResourceType::StaticGeometryMap:    return new CPoiToWorld(pEntry);
        case EResourceType::StringList:           return new CStringList(pEntry);
        case EResourceType::StringTable:          return new CStringTable(pEntry);
        case EResourceType::Texture:              return new CTexture(pEntry);
        case EResourceType::World:                return new CWorld(pEntry);
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
        case EResourceType::Animation:            pRes = CAnimationLoader::LoadANIM(rInput, pEntry);          break;
        case EResourceType::AnimEventData:        pRes = CAnimEventLoader::LoadEVNT(rInput, pEntry);          break;
        case EResourceType::AnimSet:              pRes = CAnimSetLoader::LoadANCS(rInput, pEntry);            break;
        case EResourceType::Area:                 pRes = CAreaLoader::LoadMREA(rInput, pEntry);               break;
        case EResourceType::AudioMacro:           pRes = CUnsupportedFormatLoader::LoadCAUD(rInput, pEntry);  break;
        case EResourceType::AudioGroup:           pRes = CAudioGroupLoader::LoadAGSC(rInput, pEntry);         break;
        case EResourceType::AudioLookupTable:     pRes = CAudioGroupLoader::LoadATBL(rInput, pEntry);         break;
        case EResourceType::BinaryData:           pRes = CUnsupportedFormatLoader::LoadDUMB(rInput, pEntry);  break;
        case EResourceType::Character:            pRes = CAnimSetLoader::LoadCHAR(rInput, pEntry);            break;
        case EResourceType::DependencyGroup:      pRes = CDependencyGroupLoader::LoadDGRP(rInput, pEntry);    break;
        case EResourceType::DynamicCollision:     pRes = CCollisionLoader::LoadDCLN(rInput, pEntry);          break;
        case EResourceType::Font:                 pRes = CFontLoader::LoadFONT(rInput, pEntry);               break;
        case EResourceType::GuiFrame:             pRes = CUnsupportedFormatLoader::LoadFRME(rInput, pEntry);  break;
        case EResourceType::HintSystem:           pRes = CUnsupportedFormatLoader::LoadHINT(rInput, pEntry);  break;
        case EResourceType::MapArea:              pRes = CUnsupportedFormatLoader::LoadMAPA(rInput, pEntry);  break;
        case EResourceType::MapWorld:             pRes = CUnsupportedFormatLoader::LoadMAPW(rInput, pEntry);  break;
        case EResourceType::MapUniverse:          pRes = CUnsupportedFormatLoader::LoadMAPU(rInput, pEntry);  break;
        case EResourceType::Midi:                 pRes = CUnsupportedFormatLoader::LoadCSNG(rInput, pEntry);  break;
        case EResourceType::Model:                pRes = CModelLoader::LoadCMDL(rInput, pEntry);              break;
        case EResourceType::RuleSet:              pRes = CUnsupportedFormatLoader::LoadRULE(rInput, pEntry);  break;
        case EResourceType::Scan:                 pRes = CScanLoader::LoadSCAN(rInput, pEntry);               break;
        case EResourceType::Skeleton:             pRes = CSkeletonLoader::LoadCINF(rInput, pEntry);           break;
        case EResourceType::Skin:                 pRes = CSkinLoader::LoadCSKR(rInput, pEntry);               break;
        case EResourceType::SourceAnimData:       pRes = CAnimSetLoader::LoadSAND(rInput, pEntry);            break;
        case EResourceType::StateMachine2:        pRes = CUnsupportedFormatLoader::LoadFSM2(rInput, pEntry);  break;
        case EResourceType::StaticGeometryMap:    pRes = CPoiToWorldLoader::LoadEGMC(rInput, pEntry);         break;
        case EResourceType::StringList:           pRes = CAudioGroupLoader::LoadSTLC(rInput, pEntry);         break;
        case EResourceType::StringTable:          pRes = CStringLoader::LoadSTRG(rInput, pEntry);             break;
        case EResourceType::Texture:              pRes = CTextureDecoder::LoadTXTR(rInput, pEntry);           break;
        case EResourceType::Tweaks:               pRes = CTweakLoader::LoadCTWK(rInput, pEntry);              break;
        case EResourceType::World:                pRes = CWorldLoader::LoadMLVL(rInput, pEntry);              break;

        case EResourceType::StateMachine:
            // AFSM currently unsupported
            if (pEntry->Game() <= EGame::Echoes)
                pRes = new CDependencyGroup(pEntry);
            else if (pEntry->Game() <= EGame::Corruption)
                pRes = CUnsupportedFormatLoader::LoadFSM2(rInput, pEntry);
            else if (pEntry->Game() == EGame::DKCReturns)
                pRes = CUnsupportedFormatLoader::LoadFSMC(rInput, pEntry);
            break;

        case EResourceType::BurstFireData:
        case EResourceType::Particle:
        case EResourceType::ParticleElectric:
        case EResourceType::ParticleSorted:
        case EResourceType::ParticleSpawn:
        case EResourceType::ParticleSwoosh:
        case EResourceType::ParticleDecal:
        case EResourceType::ParticleWeapon:
        case EResourceType::ParticleCollisionResponse:
        case EResourceType::ParticleTransform:
        case EResourceType::UserEvaluatorData:
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

