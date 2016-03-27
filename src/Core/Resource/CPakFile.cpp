#include "CPakFile.h"
#include <Common/Log.h>
#include <Common/types.h>
#include <FileIO/CMemoryInStream.h>
#include <FileIO/FileIO.h>

#include <zlib.h>
#include <lzo/lzo1x.h>
#include <iostream>
#include <iomanip>

CPakFile::CPakFile()
    : mpPak(nullptr)
{
}

CPakFile::CPakFile(IInputStream* pPakFile)
{
    mpPak = pPakFile;
    if (!mpPak->IsValid()) return;

    mVersion = mpPak->ReadLong();
    mpPak->Seek(0x4, SEEK_CUR);

    u32 NamedResCount = mpPak->ReadLong();
    mNamedResTable.resize(NamedResCount);

    for (u32 iName = 0; iName < NamedResCount; iName++)
    {
        SNamedResource *pRes = &mNamedResTable[iName];
        pRes->Type = CFourCC(*mpPak);
        pRes->ID = (u64) mpPak->ReadLong();
        u32 resNameLength = mpPak->ReadLong();
        pRes->Name = mpPak->ReadString(resNameLength);
    }

    u32 ResCount = mpPak->ReadLong();
    mResInfoTable.resize(ResCount);

    for (u32 iRes = 0; iRes < ResCount; iRes++)
    {
        SResInfo *pRes = &mResInfoTable[iRes];
        pRes->Compressed = (mpPak->ReadLong() != 0);
        pRes->Type = CFourCC(*mpPak);
        pRes->ID = (u64) mpPak->ReadLong();
        pRes->Size = mpPak->ReadLong();
        pRes->Offset = mpPak->ReadLong();
    }
}

CPakFile::~CPakFile()
{
    if (mpPak) delete mpPak;
}

std::vector<SNamedResource> CPakFile::NamedResources()
{
    return mNamedResTable;
}

SResInfo CPakFile::ResourceInfo(u64 AssetID, CFourCC AssetType)
{
    // TODO: figure out how the game finds assets in paks, implement similar system to speed things up
    if (mResInfoTable.empty())
        return SResInfo();

    for (u32 iRes = 0; iRes < mResInfoTable.size(); iRes++)
    {
        if (((u64) (mResInfoTable[iRes].ID & 0xFFFFFFFF) == (u64) (AssetID & 0xFFFFFFFF)) && (mResInfoTable[iRes].Type == AssetType))
            return mResInfoTable[iRes];
    }

    return SResInfo();
}

std::vector<u8>* CPakFile::Resource(u64 AssetID, CFourCC AssetType)
{
    SResInfo Info = ResourceInfo(AssetID, AssetType);

    // make sure SResInfo is valid
    if ((u64) (Info.ID & 0xFFFFFFFF) != (u64) (AssetID & 0xFFFFFFFF)) return nullptr;
    else return Resource(Info);
}

std::vector<u8>* CPakFile::Resource(SResInfo& rInfo)
{
    mpPak->Seek(rInfo.Offset, SEEK_SET);
    std::vector<u8> *pResBuf = new std::vector<u8>;

    if (rInfo.Compressed)
    {
        u32 DecmpSize = mpPak->ReadLong();
        pResBuf->resize(DecmpSize);

        std::vector<u8> CmpBuf(rInfo.Size - 4);
        mpPak->ReadBytes(&CmpBuf[0], rInfo.Size - 4);

        bool Success = Decompress(CmpBuf.data(), CmpBuf.size(), pResBuf->data(), pResBuf->size());

        if (!Success)
        {
            delete pResBuf;
            return nullptr;
        }
    }

    else
    {
        pResBuf->resize(rInfo.Size);
        mpPak->ReadBytes(pResBuf->data(), rInfo.Size);
    }

    return pResBuf;
}

bool CPakFile::Decompress(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen)
{
    if ((pSrc[0] == 0x78) && (pSrc[1] == 0xda))
    {
        // zlib
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = SrcLen;
        z.next_in = pSrc;
        z.avail_out = DstLen;
        z.next_out = pDst;

        s32 Ret = inflateInit(&z);

        if (Ret == Z_OK)
        {
            Ret = inflate(&z, Z_NO_FLUSH);

            if ((Ret == Z_OK) || (Ret == Z_STREAM_END))
                Ret = inflateEnd(&z);
        }

        if ((Ret != Z_OK) && (Ret != Z_STREAM_END)) {
            Log::Error("zlib error: " + TString::FromInt32(Ret, 0, 10));
            return false;
        }

        else return true;
    }

    else
    {
        // LZO
        lzo_uint Decmp;
        s32 Ret;
        u8 *pSrcEnd = pSrc + SrcLen;
        u8 *pDstEnd = pDst + DstLen;
        lzo_init();

        while ((pSrc < pSrcEnd) && (pDst < pDstEnd))
        {
            short BlockSize;
            memcpy(&BlockSize, pSrc, 2);
            if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian) IOUtil::SwapBytes(BlockSize);
            pSrc += 2;

            Ret = lzo1x_decompress(pSrc, BlockSize, pDst, &Decmp, LZO1X_MEM_DECOMPRESS);
            if (Ret != LZO_E_OK) break;
            pSrc += BlockSize;
            pDst += Decmp;
        }

        if (Ret != LZO_E_OK)
        {
            Log::Error("LZO error: " + TString::FromInt32(Ret, 0, 10));
            return false;
        }

        else return true;
    }

    return false;
}
