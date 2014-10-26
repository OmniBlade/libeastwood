#ifndef EASTWOOD_BASEIMAGE_H
#define	EASTWOOD_BASEIMAGE_H

#include "eastwood/Palette.h"
#include "eastwood/Surface.h"
#include "eastwood/StdDef.h"
#include "eastwood/Buffer.h"
#include "eastwood/Log.h"
#include <vector>

namespace eastwood {

class BaseImage
{
public:
    BaseImage(int width, int height, Palette palette = Palette(0)) : 
        _width(width), 
        _height(height), 
        _pixels(new Bytes(new uint8_t[(_width * _height)])), 
        _palette(0) { LOG_DEBUG("Base image created size %d", _width * _height); }
    BaseImage(const BaseImage& image);
    virtual ~BaseImage() {}
    
    //virtual Surface getSurface();
    virtual void render(Surface& surface, int xpos, int ypos);
    virtual operator uint8_t*() { return *_pixels.get(); }
    virtual operator void*() { return *_pixels.get(); }
    virtual BaseImage& operator=(const BaseImage& image);
    
    unsigned int width() { return _width; }
    unsigned int height() { return _height; }
    unsigned int size() { return _height * _width; }
    Palette palette() { return _palette; }
    
    void setPalette(Palette pal) { _palette = pal; }
protected:
    uint32_t _width;
    uint32_t _height;
    BytesPtr _pixels;
    Palette _palette;
};

} //eastwood

#endif	/* EASTWOOD_BASEIMAGE_H */

