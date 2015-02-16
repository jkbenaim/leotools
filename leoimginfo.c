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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <iconv.h>
// #include <time.h>
#include <utime.h>

#include "endian.h"
#include "leogeo.h"
#include "mfs.h"
#include "sha1.h"

void die( char *msg )
{
    fprintf( stderr, "%s\n", msg );
    exit(1);
}

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        die( "not enough arguments" );
    
    FILE *f = fopen(argv[1], "r");
    if(f == NULL)
        die( "couldn't open file" );
    
    uint8_t sysarea[232];
    uint8_t diskid[232];
    fseek(f, 0L, SEEK_SET);
    if(!fread(sysarea,232,1,f))
        die( "couldn't read system area" );
    fseek(f, 276080L, SEEK_SET);
    if(!fread(diskid,232,1,f))
        die( "couldn't read disk id" );
    
    imginfo i;
    LeoGeo_analyze_disk(&i, sysarea, diskid);
    
    printf( "SYSTEM AREA\n" );
    printf( "  Disk type     : %d (%s)\n", i.disk_type, (i.retail?"retail":"dev") );
    printf( "  IPL load address : %08x\n", i.ipl_load_address );
    printf( "  IPL load size : %d\n", i.ipl_load_size );
    printf( "  ROM end lba   : %d\n", i.rom_end_lba );
    printf( "  RAM start lba : %d\n", i.ram_start_lba );
    printf( "  RAM end lba   : %d\n", i.ram_end_lba );
    printf( "DISK ID\n" );
    printf( "  Initial code  : %s\n", i.initial_code );
    printf( "  Game version  : %d\n", i.game_version );
    printf( "  Disk number   : %d\n", i.disk_number );
    printf( "  RAM use       : %s\n", i.ram_use?"yes":"no" );
    printf( "  disk use      : %d\n", i.disk_use );
    printf( "  timestamp     : %s\n", i.manufacture_datetime );
    printf( "  company code  : %s\n", i.company_code );
    printf( "  free area     : %02x%02x%02x%02x%02x%02x (", i.free_area[0], i.free_area[1], i.free_area[2], i.free_area[3], i.free_area[4], i.free_area[5] ); 
    // if the free area contains printable ascii, print it here.
    // non-printable characters are shown as a '.'
    int n;
    for(n=0; n<6; n++)
        if( (i.free_area[n] >= 0x20) && (i.free_area[n] <= 0x7e) )
            printf( "%c", i.free_area[n] );
        else
            printf( "." );
    printf( ")\n" );
    
    // load ROM area into ram
    int rom_start_offset = (19720*24) + LeoGeo_lba_to_offset( i.disk_type, 0 );
    int rom_end_offset =   (19720*24) + LeoGeo_lba_to_offset( i.disk_type, i.rom_end_lba+1 ) - 1;
    int rom_size = rom_end_offset - rom_start_offset + 1;
    uint8_t *rom_area = calloc(1, rom_size);
    if( rom_area == NULL )
      die( "couldn't allocate memory for whole rom area" );
    fseek(f, rom_start_offset, SEEK_SET);
    if(!fread(rom_area, rom_size, 1, f))
      die( "couldn't read whole rom area" );
    
    // calculate sha1 of rom area
    int t;
    SHA1_CTX sha1context;
    unsigned char digest[SHA1_DIGEST_SIZE];
    SHA1_Init(&sha1context);
    SHA1_Update( &sha1context, rom_area, rom_size );
    SHA1_Final( &sha1context, digest );
    printf( "SHA1 of ROM area: " );
    for( t=0; t<SHA1_DIGEST_SIZE; t++ )
      printf( "%02X", digest[t] );
    printf( "\n" );
    free(rom_area);
    
    // check ram MFS
    MFS_CTX mfs_context;
    int err = MFS_ram_init( &mfs_context, f);
    printf( "MFS (RAM)\n" );
    printf( "  ram start off : %x\n", mfs_context.ram_start_offset );
    printf( "  present       : %s (%d)\n", mfs_context.valid?"yes":"no", err);
    if( mfs_context.valid )
    {
        printf( "  attr          : %02X (%s%s%s)\n", mfs_context.attr,
                (mfs_context.attr&0x80)?"V":"-",    // read-only
                (mfs_context.attr&0x40)?"U":"-",    // disable other programs reading
                (mfs_context.attr&0x20)?"R":"-"     // disable other programs writing
        );
        printf( "  type          : %d (%s)\n", mfs_context.type,
          (mfs_context.type == i.disk_type)?"match":"MISTMATCH"
        );
        printf( "  volume name   : %s\n", mfs_context.volname );
        printf( "  format date   : %s\n", mfs_context.format_datetime );
        printf( "  renewal count : %d\n", mfs_context.renewal_counter );
        printf( "  destination   : %s\n", (mfs_context.destination_code=='1')?"US":"Japan" );
        printf( "  checksum      : %04X\n", mfs_context.checksum );
        printf( "  max files     : %d\n", mfs_context.maxfiles );
        printf( "\n" );
    }
    
    if(mfs_context.header)
    {
      free(mfs_context.header);
      mfs_context.header = NULL;
    }
    fclose(f);
    return 0;
}
