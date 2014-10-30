#ifndef EASTWOOD_BASEIMAGESEQUENCE_H
#define	EASTWOOD_BASEIMAGESEQUENCE_H

#include "BaseImage.h"
#include "eastwood/Palette.h"
#include <vector>
#include <istream>

namespace eastwood {
    
class BaseImageSequence{
public:
    BaseImageSequence(Palette pal = Palette(0)) : 
        _frames(), _palette(pal) {};
    virtual ~BaseImageSequence() {};
    
    virtual BaseImage& operator[] (uint16_t i) { return _frames.at(i); }
    virtual uint16_t size() const throw() { return _frames.size(); }
    Palette palette() { return _palette; }
    void setPalette(Palette pal) { _palette = pal; }
    
protected:
    std::vector<BaseImage> _frames;
    Palette _palette;

};
    
}//eastwood

#endif	/* EASTWOOD_BASEIMAGESEQUENCE_H */

