#include "CResource.h"

// ************ STATIC ************
EResType CResource::ResTypeForExtension(CFourCC Extension)
{
    Extension = Extension.ToUpper();

    if (Extension < "FONT")
    {
        if (Extension < "CSKR")
        {
            if (Extension == "AFSM") return eStateMachine;
            if (Extension == "AGSC") return eAudioGrp;
            if (Extension == "ANCS") return eAnimSet;
            if (Extension == "ANIM") return eAnimation;
            if (Extension == "ATBL") return eAudioTable;
            if (Extension == "CAUD") return eAudioData;
            if (Extension == "CHAR") return eAnimSet;
            if (Extension == "CINF") return eSkeleton;
            if (Extension == "CMDL") return eModel;
            if (Extension == "CRSC") return eCollisionResponse;
        }
        else
        {
            if (Extension == "CSKR") return eSkin;
            if (Extension == "CSMP") return eAudioSample;
            if (Extension == "CSNG") return eMidi;
            if (Extension == "CTWK") return eTweak;
            if (Extension == "DCLN") return eCollisionMeshGroup;
            if (Extension == "DGRP") return eDependencyGroup;
            if (Extension == "DSP ") return eMusicTrack;
            if (Extension == "DUMB") return eDataDump;
            if (Extension == "ELSC") return eParticleElectric;
            if (Extension == "EVNT") return eAnimEventData;
        }
    }
    else
    {
        if (Extension < "PAK ")
        {
            if (Extension == "FONT") return eFont;
            if (Extension == "FRME") return eGuiFrame;
            if (Extension == "FSM2") return eStateMachine;
            if (Extension == "HINT") return eHintSystem;
            if (Extension == "MAPA") return eMapArea;
            if (Extension == "MAPW") return eMapWorld;
            if (Extension == "MAPU") return eMapUniverse;
            if (Extension == "MLVL") return eWorld;
            if (Extension == "MREA") return eArea;
            if (Extension == "NTWK") return eTweak;
        }
        else
        {
            if (Extension == "PAK ") return ePackFile;
            if (Extension == "PART") return eParticle;
            if (Extension == "PATH") return eNavMesh;
            if (Extension == "SAVW") return eSaveWorld;
            if (Extension == "SCAN") return eScan;
            if (Extension == "STRG") return eStringTable;
            if (Extension == "STRM") return eAudioStream;
            if (Extension == "SWHC") return eParticleSwoosh;
            if (Extension == "THP ") return eVideo;
            if (Extension == "TXTR") return eTexture;
            if (Extension == "WPSC") return eProjectile;
        }
    }

    return eInvalidResType;
}
