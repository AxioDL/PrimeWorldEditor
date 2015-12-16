#include "CPakFile.h"
#include <Common/types.h>
#include <FileIO/CMemoryInStream.h>
#include <FileIO/FileIO.h>

#include <zlib.h>
#include <lzo/lzo1x.h>
#include <iostream>
#include <iomanip>

CPakFile::CPakFile()
{
    pak = nullptr;
}

CPakFile::CPakFile(IInputStream* pakfile)
{
    pak = pakfile;
    if (!pak->IsValid()) return;

    version = pak->ReadLong();
    pak->Seek(0x4, SEEK_CUR);

    u32 namedResCount = pak->ReadLong();
    NamedResTable.resize(namedResCount);

    for (u32 n = 0; n < namedResCount; n++)
    {
        SNamedResource *res = &NamedResTable[n];
        res->resType = CFourCC(*pak);
        res->resID = (u64) pak->ReadLong();
        u32 resNameLength = pak->ReadLong();
        res->resName = pak->ReadString(resNameLength);
    }

    u32 resCount = pak->ReadLong();
    ResInfoTable.resize(resCount);

    for (u32 r = 0; r < resCount; r++)
    {
        SResInfo *res = &ResInfoTable[r];
        res->compressed = (pak->ReadLong() != 0);
        res->resType = CFourCC(*pak);
        res->resID = (u64) pak->ReadLong();
        res->size = pak->ReadLong();
        res->offset = pak->ReadLong();
    }
}

CPakFile::~CPakFile()
{
    if (pak) delete pak;
}

std::vector<SNamedResource> CPakFile::getNamedResources()
{
    return NamedResTable;
}

SResInfo CPakFile::getResourceInfo(u64 assetID, CFourCC assetType)
{
    // TODO: figure out how the game finds assets in paks, implement similar system to speed things up
    if (ResInfoTable.empty())
        return SResInfo();

    for (u32 r = 0; r < ResInfoTable.size(); r++)
    {
        if (((u64) (ResInfoTable[r].resID & 0xFFFFFFFF) == (u64) (assetID & 0xFFFFFFFF)) && (ResInfoTable[r].resType == assetType))
            return ResInfoTable[r];
    }

    return SResInfo();
}

std::vector<u8>* CPakFile::getResource(u64 assetID, CFourCC assetType)
{
    SResInfo info = getResourceInfo(assetID, assetType);

    // make sure SResInfo is valid
    if ((u64) (info.resID & 0xFFFFFFFF) != (u64) (assetID & 0xFFFFFFFF)) return nullptr;
    else return getResource(info);
}

std::vector<u8>* CPakFile::getResource(SResInfo& info)
{
    pak->Seek(info.offset, SEEK_SET);
    std::vector<u8> *res_buf = new std::vector<u8>;

    if (info.compressed)
    {
        u32 decmp_size = pak->ReadLong();
        res_buf->resize(decmp_size);

        std::vector<u8> cmp_buf(info.size - 4);
        pak->ReadBytes(&cmp_buf[0], info.size - 4);

        bool dcmp = decompress(cmp_buf.data(), cmp_buf.size(), res_buf->data(), res_buf->size());

        if (!dcmp) {
            std::cout << "Error: Unable to decompress " << info.resType.ToString() << " 0x" << std::hex << std::setw(8) << std::setfill('0') << info.resID << std::dec << "\n";
            delete res_buf;
            return nullptr;
        }
    }

    else {
        res_buf->resize(info.size);
        pak->ReadBytes(res_buf->data(), info.size);
    }

    return res_buf;
}

bool CPakFile::decompress(u8 *src, u32 src_len, u8 *dst, u32 dst_len)
{
    if ((src[0] == 0x78) && (src[1] == 0xda))
    {
        // zlib
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = src_len;
        z.next_in = src;
        z.avail_out = dst_len;
        z.next_out = dst;

        s32 ret = inflateInit(&z);

        if (ret == Z_OK)
        {
            ret = inflate(&z, Z_NO_FLUSH);

            if ((ret == Z_OK) || (ret == Z_STREAM_END))
                ret = inflateEnd(&z);
        }

        if ((ret != Z_OK) && (ret != Z_STREAM_END)) {
            std::cout << "zlib error: " << std::dec << ret << "\n";
            return false;
        }

        else return true;
    }

    else {
        // LZO
        lzo_uint decmp;
        s32 ret;
        u8 *src_end = src + src_len;
        u8 *dst_end = dst + dst_len;
        lzo_init();

        while ((src < src_end) && (dst < dst_end)) {
            short block_size;
            memcpy(&block_size, src, 2);
            if (IOUtil::kSystemEndianness == IOUtil::eLittleEndian) IOUtil::SwapBytes(block_size);
            src += 2;

            ret = lzo1x_decompress(src, block_size, dst, &decmp, LZO1X_MEM_DECOMPRESS);
            if (ret != LZO_E_OK) break;
            src += block_size;
            dst += decmp;
        }

        if (ret != LZO_E_OK) {
            std::cout << "LZO error: " << std::dec << ret << "\n";
            return false;
        }

        else return true;
    }

    return false;
}
