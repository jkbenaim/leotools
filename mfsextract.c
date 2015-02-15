// 2015 jrra

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
// #include "sha1.h"

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
    
    // check ram MFS
    MFS_CTX mfs_context;
    int err = MFS_ram_init( &mfs_context, f);
    if( mfs_context.valid && !err )
    {   
        // dump directories
        int t;
        for( t=0; t<mfs_context.maxfiles; t++ )
        {
            MFS_dir dir;
            if( (mfs_context.directory[48*t]) & 0x80 )
            {
              MFS_readdir( &dir, mfs_context.directory + 48*t );
              char temp[200];
              strncpy( temp, dir.filename, 21 );
              char *p;
              while( (p = strstr( temp, "\\" )) )
                  p[0] = '/';
              
              char diskpath[200];
              MFS_dir_pathname( &mfs_context, diskpath, 200, &dir );
              char extractpath[200];
              snprintf( extractpath, 200, ".%s", diskpath );
              mkdir( extractpath, 0755 );
              struct utimbuf ut;
              ut.modtime = MFS_date2timet( mfs_context.directory + 48*t );
              ut.actime = ut.modtime;
              utime( extractpath, &ut );
              printf( "     dir:\t%s\n", extractpath );
//               MFS_dir_fullprint( &mfs_context, &dir );
//               printf("\n");
            }
        }
        
        // dump files
        for( t=0; t<mfs_context.maxfiles; t++ )
        {
            MFS_file file;
            if( (mfs_context.directory[48*t]) & 0x40 )
            {
              MFS_readfile( &file, mfs_context.directory + 48*t );
              char filename[200];
              if( !bcmp( i.initial_code, "DRDJ", 4 ) )
              {
                // dumb fixes for DRDJ
                char dirname[200];
                strncpy( dirname, file.filename, 21 );
                char *p;
                while( (p = strstr( dirname, "\\" )) )
                    p[0] = '/';
                snprintf( filename, 200, "./%s%s.%s", mfs_context.volname, dirname, file.extension );
              }
              else
              {
                  char path[200];
                  MFS_file_pathname( &mfs_context, path, 200, &file );
                  snprintf( filename, 200, ".%s", path );
              }
              FILE *outfile = fopen( filename, "w" );
              printf( "%8d:\t%s\n", file.filesize, filename );
              uint16_t blocknum = file.fat_entry_num;
              int bytes_left = file.filesize;
              uint16_t nextblocknum = blocknum;
              int size;
              if( file.filesize != 0 ) do {
                blocknum = nextblocknum;
                nextblocknum = be16toh(*((uint16_t*)(mfs_context.fat + 2*blocknum)));
                if( nextblocknum == 0xFFFF )
                {
                  // this is the last block of the file
                  int blocksize = LeoGeo_size_of_lba( i.disk_type, i.ram_start_lba + blocknum );
                  if( bytes_left > blocksize )
                    size = blocksize;
                  else
                    size = bytes_left;
                } else {
                  // this is not the last block of the file
                  size = LeoGeo_size_of_lba( i.disk_type, i.ram_start_lba + blocknum );
                }
                bytes_left -= size;
                int blockoffset = (19720*24) + LeoGeo_lba_to_offset( i.disk_type,
                                                              i.ram_start_lba + blocknum );
                fseek( f, blockoffset, SEEK_SET );
                uint8_t buf[size];
//                 printf(" about to fread( buf, %d, 1, f ) for block %04x, bytes left: %d\n", size, blocknum, bytes_left );
                if(!fread( buf, size, 1, f ) )
                  die( "error reading block" );
                fwrite( buf, size, 1, outfile );
              } while( nextblocknum != 0xFFFF && nextblocknum != 0x0000 );
              
              if( outfile )
              {
                fclose(outfile);
                struct utimbuf ut;
                ut.modtime = MFS_date2timet( mfs_context.directory + 48*t );
                ut.actime = ut.modtime;
                utime( filename, &ut );
              }
            }
        }
    }
    
    if(mfs_context.header)
    {
      free(mfs_context.header);
      mfs_context.header = NULL;
    }
    fclose(f);
    return 0;
}
