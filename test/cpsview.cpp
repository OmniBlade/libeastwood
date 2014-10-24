#include "eastwood/IStream.h"
#include "eastwood/StdDef.h"
#include "eastwood/SDL/Surface.h"
#include "eastwood/CpsFile.h"
#include "eastwood/Log.h"
#include "SDL/SDL.h"

//Screen dimension constants CPS always these dimensions
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

SDL_Event event;

using namespace eastwood;

int main(int argc, char** argv)
{
    IStream infile;
    infile.open(argv[1]);
    if(!infile.is_open()) {
        LOG_ERROR("Could not open file.");
    }
    
    SDL_Surface* window = NULL;
    
    SDL_Init( SDL_INIT_EVERYTHING );
    window = SDL_SetVideoMode( 320, 200, 32, SDL_SWSURFACE );
    
    SDL_WM_SetCaption( "CPS Viewer", NULL );
    
    CpsFile cps(infile);
    SDL::Surface surface(cps.getSurface());
    
    SDL_BlitSurface( surface, NULL, window, NULL );
    
    SDL_Flip(window);
    
    bool quit = false;
    while(!quit) {
        while(SDL_PollEvent(&event)) {
            //If the user has Xed out the window 
            if( event.type == SDL_QUIT ) { 
                //Quit the program 
                quit = true; 
            } 
        }
    }
    
}
