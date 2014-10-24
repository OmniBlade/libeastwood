#include "eastwood/IStream.h"
#include "eastwood/StdDef.h"
#include "eastwood/Surface.h"
#include "eastwood/CpsFile.h"
#include "eastwood/Log.h"
#include "SDL/SDL.h"

//Screen dimension constants CPS always these dimensions
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

using namespace eastwood;

int main(int argc, char** argv)
{
    IStream infile;
    infile.open(argv[1]);
    if(!infile.is_open()) {
        LOG_ERROR("Could not open file.");
    }
    
    SDL_Surface* window = NULL;
    SDL_Surface* screenSurface = NULL;
    
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        LOG_ERROR("SDL could not initialize! SDL_Error %s\n", SDL_GetError());
    } else {
        window = SDL_CreateWindow("CPS Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(window == NULL){
            LOG_ERROR("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }  
    }
    
}
