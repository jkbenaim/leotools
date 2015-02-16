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

#ifndef _MFS_H_
#define _MFS_H_

#include <time.h>

typedef struct MFS_CTX {
  int ram_start_offset;
  uint8_t *header;
  uint8_t *fat;
  uint8_t *directory;
  int directory_size;
  int valid;
  int attr;
  int type;
  char volname[21];
  char format_datetime[20];     // YYYY-MM-DD HH:MM:SS\0
  uint16_t renewal_counter;
  int destination_code;
  uint32_t checksum;
  int maxfiles;
} MFS_CTX;

typedef struct MFS_file {
  uint16_t attr;
  uint16_t dir_id;
  char ccode[3];
  char gcode[5];
  uint16_t fat_entry_num;
  uint32_t filesize;
  char filename[81];
  char extension[6];
  unsigned int copy_times;
  unsigned int renewal_counter;
  char datetime[20];
  uint32_t mfs_time;
} MFS_file;

typedef struct MFS_dir {
  uint16_t attr;
  uint16_t dir_id1;     // parent directory
  char ccode[3];
  char gcode[5];
  uint16_t dir_id2;
  char filename[81];
  unsigned int renewal_counter;
  char datetime[20];
  uint32_t mfs_time;
} MFS_dir;

int MFS_ram_init( MFS_CTX *c, FILE *f );
void MFS_readdir( MFS_dir *dir, uint8_t *ent );
void MFS_dir_fullprint( MFS_CTX *c, MFS_dir *dir );
void MFS_readfile( MFS_file *file, uint8_t *ent );
void MFS_file_fullprint( MFS_CTX *c, MFS_file *file );
int MFS_isfile( uint8_t *ent );
int MFS_isdir( uint8_t *ent );
void MFS_date2string( char string[20], uint8_t *datetime );
void MFS_pathname_aux( MFS_CTX *c, char *buf, size_t buflen, uint16_t target_id );
int MFS_get_file_by_id( MFS_CTX *c, MFS_dir *dir, uint16_t target_id );
void MFS_dir_pathname( MFS_CTX *c, char *buf, size_t buflen, MFS_dir *dir );
void MFS_file_pathname( MFS_CTX *c, char *buf, size_t buflen, MFS_file *file );
time_t MFS_date2timet( uint8_t *ent );

#endif // _MFS_H_
