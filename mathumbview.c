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
#include <sys/stat.h>
#include <SDL/SDL.h>
#include "endian.h"

void die( char *msg );
int input_event_filter( const SDL_Event *event );
uint32_t rgb5551_to_argb8888( uint16_t in );

int stop = 0;

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        die( "need a file" );
    
    FILE *f;
    struct stat sb;
    if( stat( argv[1], &sb ) == -1 )
        die( "couldn't stat the file" );
    
    // Allocate memory for the file.
    uint8_t *imgData = (uint8_t*)malloc(0x480);
    if( !imgData )
        die( "couldn't allocate memory to load the file" );
    
    // Read the file.
    f = fopen( argv[1], "r" );
    if( !f )
        die( "couldn't open the file" );
    if( fread( imgData, 0x480, 1, f ) != 1 )
        die( "couldn't read the file" );
    fclose( f );
    
    
    unsigned int width, height/*, size*/;
    width = height = 24;
    
    // Init SDL.
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        die( "can't init SDL" );
    SDL_WM_SetCaption( "Mario Artist Thumbnail Viewer", "Mario Artist Thumbnail Viewer" );
    
    int scale = 1;
    SDL_Surface *screen = SDL_SetVideoMode( width*scale, height*scale, 0, 0 );
    SDL_Surface *imgSurface = SDL_CreateRGBSurface( 0, width, height, 32, 0, 0, 0, 0 );
    
    // Display the image.
    int x,y;
    uint32_t *imgPixels = imgSurface->pixels;
    for( y=0; y<height; ++y ) for( x=0; x<width; ++x )
    {
        int coord = (2*(y*width+x));
        uint16_t pixel5551 = be16toh( *(uint16_t*)(imgData+coord) );
        uint32_t pixel8888 = rgb5551_to_argb8888( pixel5551 );
        if( pixel8888 & 0xFF000000 )
            imgPixels[x + y*width] = pixel8888;
        else
            imgPixels[x+y*width] = ((x/16+y/16)&1)?0x00FFFFFF:0x00DDDDDD;
    }
    SDL_SoftStretch( imgSurface, NULL, screen, NULL );
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
    
    // Instructions
    printf( "Press Q to quit.\n" );
    
    // Wait for the escape key, q key, or close button to be pressed.
    SDL_Event event;
    while( !stop )
    {
        if( SDL_WaitEvent( &event ) ) switch( event.type )
        {
            case SDL_QUIT:
                stop=1;
                break;
            case SDL_KEYDOWN:
                switch( event.key.keysym.sym )
                {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        stop=1;
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    free( imgData );
    SDL_FreeSurface( imgSurface );
    SDL_Quit();
    return 0;
}

void die( char *msg )
{
    fprintf( stderr, "%s\n", msg );
    exit(1);
}

uint32_t rgb5551_to_argb8888( uint16_t in )
{
    //rrrrrggg ggbbbbba
    int r = ((in>>11)&0x1f)<<3;
    int g = ((in>>6 )&0x1f)<<3;
    int b = ((in>>1 )&0x1f)<<3;
    int a = in & 0x1;
    
    r += (r>>5);
    g += (g>>5);
    b += (b>>5);
    if( a == 0 )
        a = 0;
    else
        a = 0xff;
    
    uint32_t argb = (a<<24) + (r<<16) + (g<<8) + b;
//     printf( "%04X %x %x %x %08X\n", in, r, g, b, argb );
    return argb;
}
