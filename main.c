// 2015 jrra
#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include <inttypes.h>

#include "leogeo.h"
#include "mfs.h"

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
    printf( "  Disk type     : %d (%s)\n", i.disk_type&0xf, (i.disk_type&0x10?"retail":"dev") );
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
    
    fclose(f);
    return 0;
}
