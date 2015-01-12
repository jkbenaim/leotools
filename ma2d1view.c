// 2015 jrra

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

void die( char *msg );
int input_event_filter( const SDL_Event *event );

int stop = 0;
SDL_Surface *screen;

int main( int argc, char *argv[] )
{
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        die( "can't init SDL" );
    SDL_WM_SetCaption( "MA2D1 Viewer", "MA2D1 Viewer" );
    screen = SDL_SetVideoMode( 216*2, 202*2, 0, 0 );
//     SDL_SetEventFilter( input_event_filter );
        
    SDL_Event event;
    while( !stop )
    {
        if( SDL_WaitEvent( &event ) ) switch( event.type )
        {
            case SDL_QUIT:
                stop=1;
                break;
            default:
                break;
        }
        
    }
    return 0;
}

void die( char *msg )
{
    fprintf(stderr, msg);
    fprintf(stderr, "\n");
    exit(1);
}

int input_event_filter( const SDL_Event *event )
{
    switch( event->type )
    {
        case SDL_QUIT:
            stop=1;
            break;
        default:
            break;
    }
    
    return 0;
}
