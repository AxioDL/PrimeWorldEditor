#include "CompressionUtil.h"
#include "Log.h"
#include "TString.h"
#include "types.h"

#include <lzo/lzo1x.h>
#include <zlib.h>

namespace CompressionUtil
{
    TString ErrorText_zlib(s32 Error)
    {
        switch (Error)
        {
        case Z_OK:              return "Z_OK";
        case Z_STREAM_END:      return "Z_STREAM_END";
        case Z_NEED_DICT:       return "Z_NEED_DICT";
        case Z_ERRNO:           return "Z_ERRNO";
        case Z_STREAM_ERROR:    return "Z_STREAM_ERROR";
        case Z_DATA_ERROR:      return "Z_DATA_ERROR";
        case Z_MEM_ERROR:       return "Z_MEM_ERROR";
        case Z_BUF_ERROR:       return "Z_BUF_ERROR";
        case Z_VERSION_ERROR:   return "Z_VERSION_ERROR";
        default:                return "UNKNOWN ZLIB ERROR";
        }
    }

    TString ErrorText_LZO(s32 Error)
    {
        switch (Error)
        {
        case LZO_E_OK:                  return "LZO_E_OK";
        case LZO_E_ERROR:               return "LZO_E_ERROR";
        case LZO_E_EOF_NOT_FOUND:       return "LZO_E_EOF_NOT_FOUND";
        case LZO_E_INPUT_NOT_CONSUMED:  return "LZO_E_INPUT_NOT_CONSUMED";
        case LZO_E_INPUT_OVERRUN:       return "LZO_E_INPUT_OVERRUN";
        case LZO_E_INTERNAL_ERROR:      return "LZO_E_INTERNAL_ERROR";
        case LZO_E_INVALID_ALIGNMENT:   return "LZO_E_INVALID_ALIGNMENT";
        case LZO_E_INVALID_ARGUMENT:    return "LZO_E_INVALID_ARGUMENT";
        case LZO_E_LOOKBEHIND_OVERRUN:  return "LZO_E_LOOKBEHIND_OVERRUN";
        case LZO_E_NOT_COMPRESSIBLE:    return "LZO_E_NOT_COMPRESSIBLE";
        case LZO_E_NOT_YET_IMPLEMENTED: return "LZO_E_NOT_YET_IMPLEMENTED";
        case LZO_E_OUTPUT_NOT_CONSUMED: return "LZO_E_OUTPUT_NOT_CONSUMED";
        case LZO_E_OUTPUT_OVERRUN:      return "LZO_E_OUTPUT_OVERRUN";
        case LZO_E_OUT_OF_MEMORY:       return "LZO_E_OUT_OF_MEMORY";
        default:                        return "UNKNOWN LZO ERROR";
        }
    }

    // ************ DECOMPRESS ************
    bool DecompressZlib(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen, u32& rTotalOut)
    {
        // Initialize z_stream
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = SrcLen;
        z.next_in = pSrc;
        z.avail_out = DstLen;
        z.next_out = pDst;

        // Attempt decompress
        s32 Error = inflateInit(&z);

        if (!Error)
        {
            Error = inflate(&z, Z_NO_FLUSH);

            if (!Error || Error == Z_STREAM_END)
                Error = inflateEnd(&z);

            rTotalOut = z.total_out;
        }

        // Check for errors
        if (Error && Error != Z_STREAM_END)
        {
            Log::Error("zlib error: " + ErrorText_zlib(Error));
            return false;
        }

        else return true;
    }

    bool DecompressLZO(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut)
    {
        lzo_init();
        s32 Error = lzo1x_decompress(pSrc, SrcLen, pDst, &rTotalOut, LZO1X_MEM_DECOMPRESS);

        if (Error)
        {
            Log::Error("LZO error: " + ErrorText_LZO(Error));
            return false;
        }

        return true;
    }

    bool DecompressSegmentedData(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen)
    {
        u8 *pSrcEnd = pSrc + SrcLen;
        u8 *pDstEnd = pDst + DstLen;

        while ((pSrc < pSrcEnd) && (pDst < pDstEnd))
        {
            // Read size value (this method is Endian-independent)
            u8 ByteA = *pSrc++;
            u8 ByteB = *pSrc++;
            s16 Size = (ByteA << 8) | ByteB;

            u32 TotalOut;

            // Negative size denotes uncompressed data.
            if (Size < 0)
            {
                Size = -Size;
                memcpy(pDst, pSrc, Size);
                pSrc += Size;
                pDst += Size;
            }

            // If size is positive then we have compressed data.
            else
            {
                // Check for zlib magic
                u8 ByteC = pSrc[0];
                u8 ByteD = pSrc[1];
                u16 PeekMagic = (ByteC << 8) | ByteD;

                if (PeekMagic == 0x78DA || PeekMagic == 0x789C || PeekMagic == 0x7801)
                {
                    bool Success = DecompressZlib(pSrc, Size, pDst, pDstEnd - pDst, TotalOut);
                    if (!Success) return false;
                }

                // No zlib magic - this is LZO
                else
                {
                    bool Success = DecompressLZO(pSrc, Size, pDst, TotalOut);
                    if (!Success) return false;
                }

                pSrc += Size;
                pDst += TotalOut;
            }
        }

        return ((pSrc == pSrcEnd) && (pDst == pDstEnd));
    }

    // ************ COMPRESS ************
    bool CompressZlib(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen, u32& rTotalOut)
    {
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = SrcLen;
        z.next_in = pSrc;
        z.avail_out = DstLen;
        z.next_out = pDst;

        s32 Error = deflateInit(&z, 9);

        if (!Error)
        {
            Error = deflate(&z, Z_FINISH);

            if (!Error || Error == Z_STREAM_END)
                Error = deflateEnd(&z);

            rTotalOut = z.total_out;
        }

        if (Error && Error != Z_STREAM_END)
        {
            Log::Error("zlib error: " + ErrorText_zlib(Error));
            return false;
        }

        else return true;
    }

    bool CompressLZO(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut)
    {
        lzo_init();

        u8 *pWorkMem = new u8[LZO1X_999_MEM_COMPRESS];
        s32 Error = lzo1x_999_compress(pSrc, SrcLen, pDst, &rTotalOut, pWorkMem);
        delete[] pWorkMem;

        if (Error)
        {
            Log::Error("LZO error: " + ErrorText_LZO(Error));
            return false;
        }

        return true;
    }

    bool CompressSegmentedData(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut, bool IsZlib)
    {
        u8 *pSrcEnd = pSrc + SrcLen;
        u8 *pDstStart = pDst;

        while (pSrc < pSrcEnd)
        {
            // Each segment is compressed separately. Segment size should always be 0x4000 unless there's less than 0x4000 bytes left.
            u16 Size;
            u32 Remaining = pSrcEnd - pSrc;

            if (Remaining < 0x4000) Size = (u16) Remaining;
            else Size = 0x4000;

            std::vector<u8> Compressed(Size * 2);
            u32 TotalOut;

            if (IsZlib)
                CompressZlib(pSrc, Size, Compressed.data(), Compressed.size(), TotalOut);
            else
                CompressLZO(pSrc, Size, Compressed.data(), TotalOut);

            // Verify that the compressed data is actually smaller.
            if (TotalOut >= Size)
            {
                // Write negative size value to destination (which signifies uncompressed)
                *pDst++ = -Size >> 8;
                *pDst++ = -Size & 0xFF;

                // Write original uncompressed data to destination
                memcpy(pDst, pSrc, Size);
                TotalOut = Size;
            }

            // If it's not smaller, write the compressed data
            else
            {
                // Write new compressed size + data to destination
                *pDst++ = (TotalOut >> 8) & 0xFF;
                *pDst++ = (TotalOut & 0xFF);
                memcpy(pDst, Compressed.data(), TotalOut);
            }

            pSrc += Size;
            pDst += TotalOut;
        }

        rTotalOut = pDst - pDstStart;
        return true;
    }

    bool CompressZlibSegmented(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut)
    {
        return CompressSegmentedData(pSrc, SrcLen, pDst, rTotalOut, true);
    }

    bool CompressLZOSegmented(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut)
    {
        return CompressSegmentedData(pSrc, SrcLen, pDst, rTotalOut, false);
    }
}
