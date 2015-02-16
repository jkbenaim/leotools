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

#include <stdio.h>      // for snprintf
#include "endian.h"
#include "leogeo.h"

void LeoGeo_analyze_disk( imginfo *i, uint8_t sysarea[232], uint8_t diskid[232] )
{
    // system area
    i->disk_type        = sysarea[5]&0x0f;
    i->retail           = (sysarea[5]&0x10)?1:0;
    i->ipl_load_size    = be16toh(*((uint16_t*)(sysarea+0x06)));
    i->ipl_load_address = be32toh(*((uint32_t*)(sysarea+0x1c)));
    i->rom_end_lba      = be16toh(*((uint16_t*)(sysarea+0xe0)));
    i->ram_start_lba    = be16toh(*((uint16_t*)(sysarea+0xe2)));
    i->ram_end_lba      = be16toh(*((uint16_t*)(sysarea+0xe4)));
    
    // disk id
    int num;
    for(num=0; num<4; num++)
        i->initial_code[num] = diskid[num];
    i->initial_code[4] = '\0';
    i->game_version    = diskid[4];
    i->disk_number     = diskid[5];
    i->ram_use         = diskid[6];
    i->disk_use        = diskid[7];
    i->company_code[0] = diskid[0x18];
    i->company_code[1] = diskid[0x19];
    i->company_code[2] = '\0';
    for(num=0;num<6;num++)
        i->free_area[num] = diskid[0x1a+num];
    
    snprintf(i->manufacture_datetime, 20, "%04x-%02x-%02x %02x:%02x:%02x",
             be16toh(*((uint16_t*)(diskid+0x11))),      // year
             diskid[0x13],      // month
             diskid[0x14],      // day
             diskid[0x15],      // hour
             diskid[0x16],      // minute
             diskid[0x17]       // second
    );
    i->manufacture_datetime[19] = '\0';
}

int pzone_tbl1[7][16] = {
/*disk type 0*/  {267, 559, 833, 1125, 1417, 1691, 1965, 2239, 2513, 2717, 2921, 3195, 3469, 3743, 4017, 4291,},
/*disk type 1*/  {267, 559, 833, 1107, 1381, 1673, 1965, 2239, 2513, 2787, 2991, 3195, 3469, 3743, 4017, 4291,},
/*disk type 2*/  {267, 559, 833, 1107, 1381, 1655, 1929, 2221, 2513, 2787, 3061, 3265, 3469, 3743, 4017, 4291,},
/*disk type 3*/  {267, 559, 833, 1107, 1381, 1655, 1929, 2203, 2477, 2769, 3061, 3335, 3539, 3743, 4017, 4291,},
/*disk type 4*/  {267, 559, 833, 1107, 1381, 1655, 1929, 2203, 2477, 2751, 3025, 3317, 3609, 3813, 4017, 4291,},
/*disk type 5*/  {267, 559, 833, 1107, 1381, 1655, 1929, 2133, 2407, 2681, 2955, 3229, 3503, 3795, 4087, 4291,},
/*disk type 6*/  {267, 559, 833, 1107, 1381, 1655, 1929, 2133, 2337, 2611, 2885, 3159, 3433, 3707, 3999, 4291,},
};

int pzone_tbl2[7][16] = {
/*disk type 0*/  { 0,  1,  2, 14, 15,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,},
/*disk type 1*/  { 0,  1,  2,  3, 13, 14, 15,  4,  5,  6,  7,  8,  9, 10, 11, 12,},
/*disk type 2*/  { 0,  1,  2,  3,  4, 12, 13, 14, 15,  5,  6,  7,  8,  9, 10, 11,},
/*disk type 3*/  { 0,  1,  2,  3,  4,  5, 11, 12, 13, 14, 15,  6,  7,  8,  9, 10,},
/*disk type 4*/  { 0,  1,  2,  3,  4,  5,  6, 10, 11, 12, 13, 14, 15,  7,  8,  9,},
/*disk type 5*/  { 0,  1,  2,  3,  4,  5,  6,  7,  9, 10, 11, 12, 13, 14, 15,  8,},
/*disk type 6*/  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,},
};

int vzone_tbl[16] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1,
};

int block_size_per_vzone[9] = {
  19720, 18360, 17680, 16320, 14960, 13600, 12240, 10880, 9520,
};


int LeoGeo_lba_to_pzone( int type, int lba )
{
  int pzone = 0;
  int i;
  for( i=15; i>=0; i-- )
    if( lba <= pzone_tbl1[type][i] )
      pzone = pzone_tbl2[type][i];
    else
      break;
  
  return pzone;
}

int LeoGeo_pzone_to_vzone( int pzone )
{
  return vzone_tbl[ pzone ];
}

int LeoGeo_size_of_lba( int type, int lba )
{
  return block_size_per_vzone[ LeoGeo_pzone_to_vzone( LeoGeo_lba_to_pzone( type, lba ) ) ];
}

int LeoGeo_size_of_sectors( int type, int lba )
{
  return LeoGeo_size_of_lba( type, lba ) / 85;
}

int LeoGeo_lba_to_offset( int type, int lba )
{
  int offset = 0;
  int i;
  for( i = 0; i<lba; i++ )
    offset += LeoGeo_size_of_lba( type, i );
  return offset;
}
