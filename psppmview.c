// 2015 jrra

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <SDL/SDL.h>

#include "endian.h"
#include "yay1.h"

void die( char *msg );
int input_event_filter( const SDL_Event *event );
uint32_t rgb5551_to_argb8888( uint16_t in );
void redraw( SDL_Surface *screen, SDL_Surface *surface, uint8_t *imgData, unsigned int width, unsigned int height );

int stop = 0;

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        die( "need a file" );
    
    FILE *f;
    struct stat sb;
    if( stat( argv[1], &sb ) == -1 )
        die( "couldn't stat PSPPM file" );
    
    // Allocate memory for PSPPM file.
    uint8_t *psppm = (uint8_t*)malloc(sb.st_size);
    if( !psppm )
        die( "couldn't allocate memory to load PSPPM file" );
    
    // Read PSPPM file.
    f = fopen( argv[1], "r" );
    if( !f )
        die( "couldn't open PSPPM file" );
    if( fread( psppm, sb.st_size, 1, f ) != 1 )
        die( "couldn't read PSPPM file" );
    fclose( f );
    
    // Extract image info.
    unsigned int width, height, size, frames;
    width  = 216;
    height = 204;
    frames = psppm[0x481];
    size = frames*width*height*2;
    printf( "Dimensions: %dx%d, frames: %d, size: %d bytes\n", width, height, frames, size );
    
    
    // Init SDL.
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        die( "can't init SDL" );
    SDL_WM_SetCaption( "PSPPM Viewer", "PSPPM Viewer" );
    
    int scale = 1;
    SDL_Surface *screen = SDL_SetVideoMode( width*scale, height*scale, 0, 0 );
    SDL_Surface *imgSurface = SDL_CreateRGBSurface( 0, width, height, 32, 0, 0, 0, 0 );
    
    // Display the image.
    int currentFrame = 0;
    uint8_t *imgData = psppm + 0x490 + currentFrame*(216*202*2);
    redraw( screen, imgSurface, imgData, width, height );
    
    // Instructions
    printf( "Use the left and right arrow keys to switch frames,\nor press Q to quit.\n" );
    
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
                    case SDLK_LEFT:
                        if( --currentFrame == -1 )
                            currentFrame = frames - 1;
                        imgData = psppm + 0x490 + currentFrame*(216*202*2);
                        redraw( screen, imgSurface, imgData, width, height );
                        printf( "frame %d\n", currentFrame );
                        break;
                    case SDLK_RIGHT:
                        if( ++currentFrame == frames )
                            currentFrame = 0;
                        imgData = psppm + 0x490 + currentFrame*(216*202*2);
                        redraw( screen, imgSurface, imgData, width, height );
                        printf( "frame %d\n", currentFrame );
                        break;
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    free( psppm );
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

void redraw( SDL_Surface *screen, SDL_Surface *surface, uint8_t *imgData, unsigned int width, unsigned int height )
{
    int x,y;
    uint32_t *imgPixels = surface->pixels;
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
    SDL_SoftStretch( surface, NULL, screen, NULL );
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
