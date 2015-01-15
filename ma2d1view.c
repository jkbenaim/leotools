// 2015 jrra

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <SDL/SDL.h>

#include "yay1.h"

void die( char *msg );
int input_event_filter( const SDL_Event *event );
uint32_t rgb565_to_rgb888( uint16_t in );

int stop = 0;

int main( int argc, char *argv[] )
{
    if( argc < 2 )
        die( "need a file" );
    
    FILE *f;
    struct stat sb;
    if( stat( argv[1], &sb ) == -1 )
        die( "couldn't stat MA2D1 file" );
    
    // Allocate memory for MA2D1 file.
    uint8_t *ma2d1 = (uint8_t*)malloc(sb.st_size);
    if( !ma2d1 )
        die( "couldn't allocate memory to load MA2D1 file" );
    
    // Read MA2D1 file.
    f = fopen( argv[1], "r" );
    if( !f )
        die( "couldn't open MA2D1 file" );
    if( fread( ma2d1, sb.st_size, 1, f ) != 1 )
        die( "couldn't read MA2D1 file" );
    fclose( f );
    
    // Extract image info.
    char format[5];
    unsigned int width, height, size;
    sscanf( (const char*)ma2d1+0x480, "%04c%03u%03u%06u", format, &width, &height, &size );
    format[4] = '\0';
    printf( "Format: %s, dimensions %dx%d, %d bytes\n", format, width, height, size );
    
    // Decompress image (if necessary).
    uint8_t *imgData;
    if( !bcmp( format, "NCMP", 4 ) )
    {
        imgData = malloc( width*height*2 +32);
        if( !imgData )
            die( "couldn't allocate memory to decompress image" );
        if( yay1_decode( ma2d1+0x490, imgData ) )
            die( "couldn't decompress image" );
    }
    else if( !bcmp( format, "RGBA", 4 ))
    {
        // uncompressed, ezpz
        imgData = ma2d1 + 0x490;
    }
    
    
    // Init SDL.
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        die( "can't init SDL" );
    SDL_WM_SetCaption( "MA2D1 Viewer", "MA2D1 Viewer" );
    
    int scale = 1;
    SDL_Surface *screen = SDL_SetVideoMode( width*scale, height*scale, 0, 0 );
    SDL_Surface *imgSurface = SDL_CreateRGBSurface( 0, width, height, 32, 0, 0, 0, 0 );
    
    // Display the image.
    int x,y;
    uint32_t *imgPixels = imgSurface->pixels;
    for( y=0; y<height; ++y ) for( x=0; x<width; ++x )
    {
        int coord = (2*(y*width+x));
        uint16_t pixel565 = be16toh( *(uint16_t*)(imgData+coord) );
        uint32_t pixel888 = rgb565_to_rgb888( pixel565 );
        imgPixels[x + y*width] = pixel888;
    }
    SDL_SoftStretch( imgSurface, NULL, screen, NULL );
    SDL_UpdateRect( screen, 0, 0, 0, 0 );
    
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
    
    // Free imgData only if we had to allocate memory for
    // its decompressed version.
    if( !bcmp( format, "NCMP", 4 ) )
        free( imgData );
    free( ma2d1 );
    SDL_FreeSurface( imgSurface );
    SDL_Quit();
    return 0;
}

void die( char *msg )
{
    fprintf(stderr, msg);
    fprintf(stderr, "\n");
    exit(1);
}

uint32_t rgb565_to_rgb888( uint16_t in )
{
    //rrrrrggg gggbbbbb
    int r = ((in>>11)&0x1f)<<3;
    int g = ((in>>5 )&0x3f)<<2;
    int b = ((in    )&0x1f)<<3;
    
    r += (r>>5);
    g += (g>>6);
    b += (b>>5);
    return (r<<16) + (g<<8) + b;
}
