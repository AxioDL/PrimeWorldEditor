#include "types.h"
#include <lzo/lzo1x.h>
#include <cstring>

namespace CompressionUtil
{
    bool DecompressAreaLZO(u8 *src, u32 src_len, u8 *dst, u32 dst_len)
    {
        u8 *src_end = src + src_len;
        u8 *dst_end = dst + dst_len;

        lzo_init();
        lzo_uint decmp;

        while ((src < src_end) && (dst < dst_end))
        {
            u8 a = *src++;
            u8 b = *src++;
            u16 size = (a << 8) | b;

            if (size >= 0xC000)
            {
                size = 0x10000 - size;
                memcpy(dst, src, size);
                dst += size;
                src += size;
            }

            else
            {
                lzo1x_decompress(src, size, dst, &decmp, LZO1X_MEM_DECOMPRESS);
                src += size;
                dst += decmp;
            }
        }

        return ((src == src_end) && (dst == dst_end));
    }
}
