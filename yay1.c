/*************************************************************************
 *   leotools
 *   Copyright (C) 2015 jkbenaim
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <inttypes.h>
#include <strings.h>
#include "endian.h"
#include "yay1.h"

uint32_t yay1_get_size(uint8_t *s)
{
    return be32toh( *(uint32_t*)(s+4) );
}

int yay1_decode(uint8_t *s, uint8_t *d)
{
    if( bcmp( s, "Yay1", 4 ) )
    {
        return -1;
    }
    
    uint8_t *endOfDest, *linkTable, *chunks;
    int bitsLeft = 0;
    int32_t bits = 0;
    
    uint32_t decodedSize     = yay1_get_size( s );
    uint32_t linkTableOffset = be32toh( *(uint32_t*)(s+8) );
    uint32_t chunksOffset = be32toh( *(uint32_t*)(s+0xC) );
    endOfDest = d + decodedSize;
    linkTable = s + linkTableOffset;
    chunks    = s + chunksOffset;
    
    s += 0x10;
    
    do {
        if( bitsLeft == 0 )
        {
            bitsLeft = 32;
            bits = be32toh( *(uint32_t*) s );
            s += 4;
        }
        if( bits >= 0 )
        {
            uint16_t codeword = be16toh( *(uint16_t*)linkTable );
            linkTable += 2;
            unsigned int count = codeword >> 12;
            unsigned int offset = codeword & 0xfff;
            
            uint8_t *backReference = d - offset;
            if( count == 0 )
                count = *(chunks++) + 0x12;
            else
                count += 2;
            
            do {
                count--;
                *(d++) = *(backReference++ - 1);
            } while( count != 0 );
        } else {
            *(d++) = *(chunks++);
        }
        bitsLeft--;
        bits<<=1;
    } while( d < endOfDest );
    
    return 0;
}
