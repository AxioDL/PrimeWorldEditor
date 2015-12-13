#ifndef CTEXTUREENCODER_H
#define CTEXTUREENCODER_H

#include "../CTexture.h"
#include <Core/TResPtr.h>

// Class contains basic functionality right now - only supports directly converting DXT1 to CMPR
// More advanced functions (including actual encoding!) coming later
class CTextureEncoder
{
    TResPtr<CTexture> mpTexture;
    ETexelFormat mSourceFormat;
    ETexelFormat mOutputFormat;

    CTextureEncoder();
    void WriteTXTR(COutputStream& TXTR);
    void DetermineBestOutputFormat();
    void ReadSubBlockCMPR(CInputStream& Source, COutputStream& Dest);

public:
    static void EncodeTXTR(COutputStream& TXTR, CTexture *pTex);
    static void EncodeTXTR(COutputStream& TXTR, CTexture *pTex, ETexelFormat OutputFormat);
    static ETexelFormat GetGXFormat(ETexelFormat Format);
    static ETexelFormat GetFormat(ETexelFormat Format);
};

#endif // CTEXTUREENCODER_H
