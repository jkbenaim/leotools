/* Yay0 decompression */
/* original code (c) spinout I think */

#include <n64tool.h>

int
yay0_get_size(u8 *s)
{
    return U32(s + 4);
}

void
yay0_decode(u8 *s, u8 *d)
{
    int i, j, k, p, q, cnt;

    i = U32(s + 4);    // size of decoded data
    j = U32(s + 8);    // link table
    k = U32(s + 12);   // byte chunks and count modifiers

    q = 0;         // current offset in dest buffer
    cnt = 0;       // mask bit counter
    p = 16;        // current offset in mask table

    unsigned long r22 = 0, r5;

    do
    {
        // if all bits are done, get next mask
        if(cnt == 0)
        {
            // read word from mask data block
            r22 = *(unsigned long *)(s + p);
            p += 4;
            cnt = 32;   // bit counter
        }
        // if next bit is set, chunk is non-linked
        if(r22 & 0x80000000)
        {
            // get next byte
            *(unsigned char *)(d + q) = *(unsigned char *)(s + k);
            k++, q++;
        }
        // do copy, otherwise
        else
        {
            // read 16-bit from link table
            int r26 = *(unsigned short *)(s + j);
            j += 2;
            // 'offset'
            int r25 = q - (r26 & 0xfff);
            // 'count'
            int r30 = r26 >> 12;
            if(r30 == 0)
            {
                // get 'count' modifier
                r5 = *(unsigned char *)(s + k);
                k++;
                r30 = r5 + 18;
            }
            else r30 += 2;
            // do block copy
            unsigned char *pt = ((unsigned char*)d) + r25;
            int z;
            for(z=0; z<r30; z++)
            {
                *(unsigned char *)(d + q) = *(unsigned char *)(pt - 1);
                q++, pt++;
            }
        }
        // next bit in mask
        r22 <<= 1;
        cnt--;

    } while(q < i);
}
