// 2015 jrra

#ifndef _LEOGEO_H_
#define _LEOGEO_H_

#include <inttypes.h>

typedef struct imginfo {
    // from system area
    unsigned int disk_type;
    int retail;
    uint32_t ipl_load_address;
    unsigned int ipl_load_size;
    unsigned int rom_end_lba;
    unsigned int ram_start_lba;
    unsigned int ram_end_lba;
    
    // from disk id
    char initial_code[5];
    unsigned int game_version;
    unsigned int disk_number;
    unsigned int ram_use;
    unsigned int disk_use;
    char manufacture_datetime[20];
    char company_code[3];
    uint8_t free_area[6];
} imginfo;

extern void LeoGeo_analyze_disk( imginfo *i, uint8_t sysarea[232], uint8_t diskid[232] );
extern int LeoGeo_lba_to_pzone( int type, int lba );
extern int LeoGeo_pzone_to_vzone( int pzone );
extern int LeoGeo_size_of_lba( int type, int lba );
extern int LeoGeo_size_of_sectors( int type, int lba );
extern int LeoGeo_lba_to_offset( int type, int lba );

#endif  // _LEOGEO_H_
