#include "UICommon.h"

namespace UICommon
{

QMap<QString,QString> FilterMap = {
    { "AFSM", "AI Finite State Machine (*.AFSM)" },
    { "ANIM", "Animation (*.ANIM)" },
    { "ANCS", "Animation Character Set (*.ANCS)" },
    { "AGSC", "Audio Group (*.AGSC)" },
    { "ATBL", "Audio Lookup Table (*.ATBL)" },
    { "CAUD", "Audio Metadata (*.CAUD)" },
    { "CHAR", "Character (*.CHAR)" },
    { "CINF", "Skeleton (*.CINF)" },
    { "CMDL", "Model (*.CMDL)" },
    { "CRSC", "Collision Response Data (*.CRSC)" },
    { "CSKR", "Skin Rules (*.CSKR)" },
    { "CSMP", "Audio Sample (*.CSMP)" },
    { "CSNG", "MIDI Data (*.CSNG)" },
    { "CTWK", "Tweaks (*.CTWK)" },
    { "DCLN", "Collision Mesh (*.DCLN)" },
    { "DGRP", "Dependency Group (*.DGRP)" },
    { "DPSC", "Decal (*.DPSC)" },
    { "DSP",  "Music Track (*.DSP)" },
    { "DUMB", "Binary Data Dump (*.DUMB)" },
    { "ELSC", "Electric Particle (*.ELSC)" },
    { "EVNT", "Animation Event Data (*.EVNT)" },
    { "FRME", "GUI Frame (*.FRME)" },
    { "FSM2", "AI Finite State Machine (*.FSM2)" },
    { "FONT", "Font (*.FONT)" },
    { "HINT", "Hint System Data (*.HINT)" },
    { "MAPA", "Area Map (*.MAPA)" },
    { "MAPW", "World Map (*.MAPW)" },
    { "MAPU", "Universe Map (*.MAPU)" },
    { "MLVL", "World (*.MLVL)" },
    { "MREA", "Area (*.MREA)" },
    { "NTWK", "Tweaks (*.NTWK)" },
    { "PATH", "AI Navigation Mesh (*.PATH)" },
    { "PAK",  "Pack File (*.pak)" },
    { "PART", "Particle (*.PART)" },
    { "SAVW", "World Save Data (*.SAVW)" },
    { "SCAN", "Scannable Object Info (*.SCAN)" },
    { "STRG", "String Table (*.STRG)" },
    { "STRM", "Audio Stream (*.STRM)" },
    { "SWHC", "Swoosh Particle (*.SWHC)" },
    { "THP",  "Video (*.thp)" },
    { "TXTR", "Texture (*.TXTR)" },
    { "WPSC", "Projectile (*.WPSC)" }
};

QString ExtensionFilterString(const QString& extension)
{
    QMap<QString,QString>::const_iterator it = FilterMap.find(extension);

    if (it != FilterMap.end())
        return it.value();
    else
        return "Unknown Extension (*." + extension + ")";

    /*if (extension.isEmpty()) return gskInvalidFilterString;
    extension = extension.toUpper();

    switch (extension[0])
    {
    case 'A':
        if (extension == "AFSM") return "AI Finite State Machine (*.AFSM)";
        if (extension == "ANIM") return "Animation (*.ANIM)";
        if (extension == "ANCS") return "Animation Character Set (*.ANCS)";
        if (extension == "AGSC") return "Audio Group (*.AGSC)";
        if (extension == "ATBL") return "Audio Lookup Table (*.ATBL)";
        break;

    case 'C':
        if (extension[1] <= 'R')
        {
            if (extension == "CAUD") return "Audio Metadata (*.CAUD)";
            if (extension == "CHAR") return "Character (*.CHAR)";
            if (extension == "CINF") return "Skeleton (*.CINF)";
            if (extension == "CMDL") return "Model (*.CMDL)";
            if (extension == "CRSC") return "Collision Response Data (*.CRSC)";
        }
        else
        {
            if (extension == "CSKR") return "Skin Rules (*.CSKR)";
            if (extension == "CSMP") return "Audio Sample (*.CSMP)";
            if (extension == "CSNG") return "MIDI Data (*.CSNG)";
            if (extension == "CTWK") return "Tweaks (*.CTWK)";
        }
        break;

    case 'D':
        if (extension == "DCLN") return "Collision Mesh (*.DCLN)";
        if (extension == "DGRP") return "Dependency Group (*.DGRP)";
        if (extension == "DPSC") return "Decal (*.DPSC)";
        if (extension == "DSP")  return "Music Track (*.DSP)";
        if (extension == "DUMB") return "Binary Data Dump (*.DUMB)";
        break;

    case 'E':
        if (extension == "ELSC") return "Electric Particle (*.ELSC)";
        if (extension == "EVNT") return "Animation Event Data (*.EVNT)";
        break;

    case 'F':
        if (extension == "FRME") return "GUI Frame (*.FRME)";
        if (extension == "FSM2") return "AI Finite State Machine (*.FSM2)";
        if (extension == "FONT") return "Font (*.FONT)";
        break;

    case 'H':
        if (extension == "HINT") return "Hint System Data (*.HINT)";
        break;

    case 'M':
        if (extension == "MAPA") return "Area Map (*.MAPA)";
        if (extension == "MAPW") return "World Map (*.MAPW)";
        if (extension == "MAPU") return "Universe Map (*.MAPU)";
        if (extension == "MLVL") return "World (*.MLVL)";
        if (extension == "MREA") return "Area (*.MREA)";
        break;

    case 'N':
        if (extension == "NTWK") return "Tweaks (*.ntwk)";
        break;

    case 'P':
        if (extension == "PATH") return "AI Navigation Mesh (*.PATH)";
        if (extension == "PAK")  return "Pack File (*.pak)";
        if (extension == "PART") return "Particle (*.PART)";
        break;

    case 'S':
        if (extension == "SAVW") return "World Save Data (*.SAVW)";
        if (extension == "SCAN") return "Scannable Object Info (*.SCAN)";
        if (extension == "STRG") return "String Table (*.STRG)";
        if (extension == "STRM") return "Audio Stream (*.STRM)";
        if (extension == "SWHC") return "Swoosh Particle (*.SWHC)";
        break;

    case 'T':
        if (extension == "THP")  return "Video (*.thp)";
        if (extension == "TXTR") return "Texture (*.TXTR)";
        break;

    case 'W':
        if (extension == "WPSC") return "Projectile (*.WPSC)";
        break;
    }

    return gskInvalidFilterString;*/
}

}
