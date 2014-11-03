#include "eastwood/ArcIStream.h"
#include "eastwood/StdDef.h"
#include "eastwood/SDL/Surface.h"
#include "eastwood/WsaFile.h"
#include "eastwood/Log.h"
#include "SDL/SDL.h"
#include "eastwood/PalFile.h"

SDL_Event event;

using namespace eastwood;

int main(int argc, char** argv)
{
    LOG_DEBUG("Starting WSA Viewer");
    ArcIStream infile;
    infile.open("ibm.pal", std::ios_base::in | std::ios_base::binary);
    PalFile pal(infile);
    Palette palette = pal.getPalette();
    infile.close();
    
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
    LOG_INFO("Loading WSA file %s", argv[1]);
    WsaFile wsa(infile);
    frames = wsa.size();
    
    window = SDL_SetVideoMode(wsa[0].width(), wsa[0].height(), 32, SDL_SWSURFACE );
    
    SDL_WM_SetCaption( "WSA Viewer", NULL );
    
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
        surface = wsa[frame].getSurface(wsa.palette());
        SDL_BlitSurface( surface, NULL, window, NULL );
        SDL_Flip(window);
        SDL_Delay(250);
    }
    
}
