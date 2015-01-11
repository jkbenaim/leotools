// 2015 jrra
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include "leogeo.h"
#include "mfs.h"

int MFS_ram_init( MFS_CTX *c, FILE *f )
{
    c->valid = 0;
    c->header = NULL;
    uint8_t sysarea[232];
    uint8_t diskid[232];
    fseek(f, 0L, SEEK_SET);
    if(!fread(sysarea,232,1,f))
        return -1;
    fseek(f, 276080L, SEEK_SET);
    if(!fread(diskid,232,1,f))
        return -2;
    
    imginfo i;
    LeoGeo_analyze_disk(&i, sysarea, diskid);
    
    // This check is mandated by the MFS spec,
    // but some games don't obey it...
//     if(!i.ram_use)
//       return -3;
    
    int ram_start_offset = (19720*24) + LeoGeo_lba_to_offset( i.disk_type,
                                                              i.ram_start_lba );
    c->ram_start_offset = ram_start_offset;
    int ram_blocksize = LeoGeo_size_of_lba( i.disk_type, i.ram_start_lba );
    
    // read 6 blocks (3 MFS primary + 3 MFS backup, each half should be identical)
    c->header = calloc(6, ram_blocksize);
    fseek(f, ram_start_offset, SEEK_SET);
    if(!fread(c->header,ram_blocksize*6,1,f))
    {
      free( c->header );
      c->header = NULL;
      return -4;
    }
    
    c->fat = c->header + 60;
    c->directory = c->fat + 5748;
    c->directory_size = ram_blocksize*3 - 60 - 5748;
    
    // check that fs magic is present
    int count;
    for( count=0; count<10; count++ )
      if( c->header[count] != (uint8_t)"64dd-Multi"[count] )
      {
        free( c->header );
        c->header = NULL;
        return -5;
      }
    
    // check that each half is identical
    for( count=0; count<(ram_blocksize*3); count++ )
      if( c->header[count] != c->header[count+(ram_blocksize*3)] )
      {
//         free( c->header );
//         c->header = NULL;
//         return -6;
      }
      
    // check that checksum is OK
    uint32_t checksum = 0;
    for( count = 0; count<(ram_blocksize*3)/4; count++ )
      checksum ^= *((uint32_t*)(c->header+count*4));
    if( checksum != 0 )
      {
//         free( c->header );
//         c->header = NULL;
//         return -7;
      }
    
    c->valid = 1;
    c->attr = c->header[0xe];
    c->type = c->header[0xf];
    for( count=0; count<21; count++ )
      c->volname[count] = c->header[0x10 + count];
    c->volname[20] = '\0';
    
    MFS_date2string( c->format_datetime, c->header + 0x24 );
    
    c->renewal_counter = be16toh(*((uint16_t*)(c->header+0x28)));
    c->checksum = be32toh(*((uint32_t*)(c->header+0x2c)));
    
    c->maxfiles = (LeoGeo_size_of_lba( i.disk_type, i.ram_start_lba )*3 - 5808)/48;
    return 0;
}

void MFS_readdir( MFS_dir *dir, uint8_t *ent )
{
    // first step: convert filename to UTF-8
    char sjis_name[21];
    memcpy( sjis_name, ent+0x10, 20 );
    sjis_name[20] = '\0';
    iconv_t cd = iconv_open( "UTF-8", "SHIFT_JIS" );
    char *in = sjis_name;
    char *out = dir->filename;
    size_t ibl = 20;
    size_t obl = 80;
    iconv( cd, &in, &ibl, &out, &obl );
    iconv_close( cd );
    
    dir->attr = be16toh(*((uint16_t*)(ent+0)));
    dir->dir_id1 = be16toh(*((uint16_t*)(ent+2)));
    memcpy( dir->ccode, ent+4, 2 );
    dir->ccode[2] = '\0';
    memcpy( dir->gcode, ent+6, 4 );
    dir->gcode[4] = '\0';
    dir->dir_id2 = be16toh(*((uint16_t*)(ent+0x0a)));
    dir->renewal_counter = ent[0x2a];
    MFS_date2string( dir->datetime, ent+0x2c );
    dir->mfs_time = *((uint16_t*)(ent+0x2c));
}

void MFS_dir_fullprint( MFS_CTX *c, MFS_dir *dir )
{
    char pathname[1000];
    MFS_dir_pathname( c, pathname, 1000, dir );
    
    printf( "  attr          : %04X (%s%s%s%s%s%s%s)\n", 
            dir->attr,
            (dir->attr)&0x8000 ? "d" : "-",
            (dir->attr)&0x4000 ? "f" : "-",
            (dir->attr)&0x2000 ? "W" : "-",
            (dir->attr)&0x1000 ? "R" : "-",
            (dir->attr)&0x0800 ? "h" : "-",
            (dir->attr)&0x0400 ? "e" : "-",
            (dir->attr)&0x0200 ? "L" : "-"
    );
    printf( "  dir name      : %s\n", dir->filename );
    printf( "  parent dir id : %04X\n", dir->dir_id1 );
    printf( "  fullpath      : %s\n", pathname );
    printf( "  company code  : %s\n", dir->ccode );
    printf( "  game code     : %s\n", dir->gcode );
    printf( "  dir id        : %04X\n", dir->dir_id2 );
    printf( "  renewal count : %d\n", dir->renewal_counter );
    printf( "  creation date : %s\n", dir->datetime );
}

void MFS_date2string( char string[20], uint8_t *datetime )
{
    int year = 1996 + ((datetime[0] & 0xfe)>>1);
    int month = ((datetime[0] & 0x01)<<3) + ((datetime[1] & 0xe0)>>5);
    int day = (datetime[1] & 0x1f);
    int hour = ((datetime[2] & 0xf8)>>3);
    int minute = ((datetime[2] & 0x07)<<3) + ((datetime[3] & 0xe0)>>5);
    int second = ((datetime[3] & 0x1f)<<1);
      
    snprintf(string, 20, "%04d-%02d-%02d %02d:%02d:%02d",
             year,
             month,
             day,
             hour,
             minute,
             second
    );
    string[19] = '\0';
}

time_t MFS_date2timet( uint8_t *ent )
{
    uint8_t *datetime = ent + 0x2c;
    int year = 1996 + ((datetime[0] & 0xfe)>>1);
    int month = ((datetime[0] & 0x01)<<3) + ((datetime[1] & 0xe0)>>5);
    int day = (datetime[1] & 0x1f);
    int hour = ((datetime[2] & 0xf8)>>3);
    int minute = ((datetime[2] & 0x07)<<3) + ((datetime[3] & 0xe0)>>5);
    int second = ((datetime[3] & 0x1f)<<1);
    
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;      // 0-11
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
//     tm.isdst = 0;
    return mktime( &tm );
}

void MFS_readfile( MFS_file *file, uint8_t *ent )
{
    // first step: convert filename to UTF-8
    char sjis_name[21];
    memcpy( sjis_name, ent+0x10, 20 );
    sjis_name[20] = '\0';
    iconv_t cd = iconv_open( "UTF-8", "SHIFT_JIS" );
    char *in = sjis_name;
    char *out = file->filename;
    size_t ibl = 20;
    size_t obl = 80;
    iconv( cd, &in, &ibl, &out, &obl );
    iconv_close( cd );
    
    file->attr = be16toh(*((uint16_t*)(ent+0)));
    file->dir_id = be16toh(*((uint16_t*)(ent+2)));
    memcpy( file->ccode, ent+4, 2 );
    file->ccode[2] = '\0';
    memcpy( file->gcode, ent+6, 4 );
    file->gcode[4] = '\0';
    file->fat_entry_num = be16toh(*((uint16_t*)(ent+0x0a)));
    file->filesize = be32toh(*((uint32_t*)(ent+0x0c)));
    memcpy( file->extension, ent+0x24, 5 );
    file->extension[5] = '\0';
    file->copy_times = ent[0x29];
    file->renewal_counter = ent[0x2a];  
    MFS_date2string( file->datetime, ent+0x2c );
    file->mfs_time = *((uint16_t*)(ent+0x2c));
}

void MFS_file_fullprint( MFS_CTX *c, MFS_file *file )
{
    char pathname[1000];
    MFS_file_pathname( c, pathname, 1000, file );
    
    printf( "  attr          : %04X (%s%s%s%s%s%s%s)\n", 
            file->attr,
            (file->attr)&0x8000 ? "d" : "-",
            (file->attr)&0x4000 ? "f" : "-",
            (file->attr)&0x2000 ? "W" : "-",
            (file->attr)&0x1000 ? "R" : "-",
            (file->attr)&0x0800 ? "h" : "-",
            (file->attr)&0x0400 ? "e" : "-",
            (file->attr)&0x0200 ? "L" : "-"
    );
    printf( "  file name     : %s.%s\n", file->filename, file->extension );
    printf( "  parent dir id : %04X\n", file->dir_id );
    printf( "  fullpath      : %s\n", pathname );
    printf( "  company code  : %s\n", file->ccode );
    printf( "  game code     : %s\n", file->gcode );
    printf( "  fat entry     : %04X\n", file->fat_entry_num );
    printf( "  file size     : %d\n", file->filesize );
    printf( "  copy times    : %d\n", file->copy_times );
    printf( "  renewal count : %d\n", file->renewal_counter );
    printf( "  creation date : %s\n", file->datetime );
}

void MFS_dir_pathname( MFS_CTX *c, char *buf, size_t buflen, MFS_dir *dir )
{
    MFS_pathname_aux( c, buf, buflen, dir->dir_id2 );
}

void MFS_file_pathname( MFS_CTX *c, char *buf, size_t buflen, MFS_file *file )
{
    MFS_pathname_aux( c, buf, buflen, file->dir_id );
    char parentpath[buflen];
    strncpy( parentpath, buf, buflen );
    snprintf( buf, buflen, "%s/%s.%s", parentpath, file->filename, file->extension );
}

void MFS_pathname_aux( MFS_CTX *c, char *buf, size_t buflen, uint16_t target_id )
{
    switch( target_id )
    {
        case 0xFFFE:
            // this is the root directory's parent
            memset( buf, 0, buflen );
            break;
        case 0:
            // root
            memset( buf, 0, buflen );
            break;
        default: 
        {
            MFS_dir dir;
            MFS_get_file_by_id( c, &dir, target_id );
            char parentpath[buflen];
            MFS_pathname_aux( c, parentpath, buflen, dir.dir_id1 );
            snprintf( buf, buflen, "%s/%s", parentpath, dir.filename );
            break;
        }
    }
}

int MFS_get_file_by_id( MFS_CTX *c, MFS_dir *dir, uint16_t target_id )
{
    int filenum;
    for(filenum=0; filenum<c->maxfiles; filenum++)
    {
        MFS_readdir( dir, c->directory + 48*filenum );
        if( dir->dir_id2 == target_id )
            return 0;
    }
    
    return -1;
}

int MFS_isfile( uint8_t *ent )
{
  return ent[0]&0x40 ? 1 : 0;
}

int MFS_isdir( uint8_t *ent )
{
  return ent[0]&0x80 ? 1 : 0;
}
