#include "eastwood/ArcIStream.h"
#include "eastwood/StdDef.h"
#include "eastwood/SDL/Surface.h"
#include "eastwood/CnCShpFile.h"
#include "eastwood/CpsFile.h"
#include "eastwood/Palette.h"
#include "eastwood/Log.h"
#include "SDL/SDL.h"

SDL_Event event;

using namespace eastwood;

int main(int argc, char** argv)
{
    LOG_DEBUG("Starting WSA Viewer");
    ArcIStream infile;
    infile.open("palette.cps", std::ios_base::in | std::ios_base::binary);
    CpsFile cps(infile);
    infile.close();
    Palette palette = cps.getPalette();
    infile.open(argv[1], std::ios_base::in | std::ios_base::binary);
    if(!infile.is_open()) {
        LOG_ERROR("Could not open file.");
        return -1;
    }
    
    //track the current frame
    uint32_t frame = 0;
    uint32_t frames = 0;
    
    SDL_Surface* window = NULL;
    
    SDL_Init( SDL_INIT_EVERYTHING );
    LOG_INFO("Loading Shp file %s", argv[1]);
    CnCShpFile shp(infile);
    frames = shp.size();
    
    window = SDL_SetVideoMode(100, 100, 32, SDL_SWSURFACE );
    
    SDL_WM_SetCaption( "Shp Viewer", NULL );
    
    SDL::Surface surface;
    
    bool quit = false;
    while(!quit) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_KEYDOWN){
                switch( event.key.keysym.sym ) { 
                case SDLK_LEFT: 
                    frame = (frame - 1) % frames; 
                    break; 
                case SDLK_RIGHT: 
                    frame = (frame + 1) % frames; 
                    break;
                default:
                    break;
                }
            }
            //If the user has Xed out the window 
            if( event.type == SDL_QUIT ) { 
                //Quit the program 
                quit = true; 
            }
        }
        surface = shp[frame].getSurface(palette);
        SDL_BlitSurface( surface, NULL, window, NULL );
        SDL_Flip(window);
        SDL_Delay(250);
    }
    
}
