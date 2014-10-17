#include "eastwood/BaseImage.h"

namespace eastwood {

Surface BaseImage::getSurface()
{
    Surface surf(_width, _height, 8, _palette);
    memcpy(surf, &_bitmap.at(0), _bitmap.size());
    
    return surf;
}

//simple straight blit to surface
void BaseImage::render(Surface& surface, int xpos, int ypos)
{
    //don't waste time if image isn't within surface boundries
    if(xpos > surface.width() || ypos > surface.height()) return;
    
    uint8_t* surf = surface;
    uint8_t* img = &_bitmap.at(0);
    uint32_t start = xpos + surface.pitch() * ypos;
    uint32_t size = xpos + _width > surface.width() ? 
                                            xpos + _width - surface.width() :
                                            _width;
    uint32_t count = ypos + _height > surface.height() ?
                                            ypos + _height - surface.height() :
                                            _height;
    surf += start;
    while(count--){
        memcpy(surf, img, size);
        surf += surface.pitch();
        img += _width;
    }
}

} //eastwood
