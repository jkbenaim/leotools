// 2015 jrra
#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include <inttypes.h>

#include "leogeo.h"
#include "mfs.h"
#include "sha1.h"

void die( char* msg )
{
    fprintf(stderr, msg);
    fprintf(stderr, "\n");
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
    printf( "  free area     : %02x%02x%02x%02x%02x%02x\n", i.free_area[0], i.free_area[1], i.free_area[2], i.free_area[3], i.free_area[4], i.free_area[5] ); 
    
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
    SHA1_CTX context;
    unsigned char digest[SHA1_DIGEST_SIZE];
    SHA1_Init(&context);
    SHA1_Update( &context, rom_area, rom_size );
    SHA1_Final( &context, digest );
    printf( "SHA1 of ROM area: " );
    int t;
    for( t=0; t<SHA1_DIGEST_SIZE; t++ )
      printf( "%02X", digest[t] );
    printf( "\n" );
    
    free(rom_area);
    fclose(f);
    return 0;
}
