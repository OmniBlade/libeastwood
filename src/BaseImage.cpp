#include "eastwood/BaseImage.h"

namespace eastwood {

BaseImage::BaseImage(const BaseImage& image) :
    _width(image._width),
    _height(image._height),
    _pixels(new Bytes(new uint8_t[(_width * _height)])),
    _palette(image._palette)
{
    uint8_t* src = *image._pixels.get();
    uint8_t* dest = *_pixels.get();
    int count = _width * _height;
    for(int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

BaseImage& BaseImage::operator =(const BaseImage& image)
{
    _width = image._width;
    _height = image._height;
    _palette = image._palette;
    uint8_t* src = *image._pixels.get();
    uint8_t* dest = *_pixels.get();
    int count = _width * _height;
    for(int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
    
    return *this;
}

Surface BaseImage::getSurface()
{
    LOG_DEBUG("Getting surface from %d x %d image", _width, _height);
    
    Surface surf(_width, _height, 8, _palette);
    memcpy(surf, *_pixels.get(), _height * _width);
    
    return surf;
}

Surface BaseImage::getSurface(Palette pal)
{
    LOG_DEBUG("Getting set pal surface from %d x %d image", _width, _height);
    
    Surface surf(_width, _height, 8, pal);
    memcpy(surf, *_pixels.get(), _height * _width);
    
    return surf;
}

//simple straight blit to surface
void BaseImage::render(Surface& surface, int xpos, int ypos)
{
    //don't waste time if image isn't within surface boundries
    if(xpos > surface.width() || ypos > surface.height()) return;
    
    uint8_t* surf = surface;
    uint8_t* img = *_pixels.get();
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
