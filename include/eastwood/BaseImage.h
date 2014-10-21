#ifndef EASTWOOD_BASEIMAGE_H
#define	EASTWOOD_BASEIMAGE_H

#include "eastwood/Palette.h"
#include "eastwood/Surface.h"
#include "eastwood/StdDef.h"

#include <vector>

namespace eastwood {

class BaseImage
{
public:
    BaseImage() : _width(0), _height(0), _bitmap(0), _palette(0) {}
    BaseImage(int width, int height, Palette palette = Palette(0)) : 
        _width(width), 
        _height(height), 
        _bitmap(width * height), 
        _palette(0) {}
    virtual ~BaseImage() {}
    
    //virtual Surface getSurface();
    virtual void render(Surface& surface, int xpos, int ypos);
    virtual operator uint8_t*() { return &_bitmap.at(0); }
    virtual operator void*() { return &_bitmap.at(0); }
    
    void setPalette(Palette pal) { _palette = pal; }
    void setDimensions(unsigned int width, unsigned int height);
protected:
    unsigned int _width;
    unsigned int _height;
    std::vector<uint8_t> _bitmap;
    Palette _palette;
};

} //eastwood

#endif	/* EASTWOOD_BASEIMAGE_H */

