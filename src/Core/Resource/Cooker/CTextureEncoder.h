#ifndef CTEXTUREENCODER_H
#define CTEXTUREENCODER_H

#include "Core/Resource/CTexture.h"
#include "Core/Resource/TResPtr.h"

// Class contains basic functionality right now - only supports directly converting DXT1 to CMPR
// More advanced functions (including actual encoding!) coming later
class CTextureEncoder
{
    TResPtr<CTexture> mpTexture;
    ETexelFormat mSourceFormat;
    ETexelFormat mOutputFormat;

    CTextureEncoder();
    void WriteTXTR(IOutputStream& TXTR);
    void DetermineBestOutputFormat();
    void ReadSubBlockCMPR(IInputStream& Source, IOutputStream& Dest);

public:
    static void EncodeTXTR(IOutputStream& TXTR, CTexture *pTex);
    static void EncodeTXTR(IOutputStream& TXTR, CTexture *pTex, ETexelFormat OutputFormat);
    static ETexelFormat GetGXFormat(ETexelFormat Format);
    static ETexelFormat GetFormat(ETexelFormat Format);
};

#endif // CTEXTUREENCODER_H
