#include "eastwood/StdDef.h"
#include "eastwood/Palette.h"

#include <cmath>

namespace eastwood {

Palette::Palette(uint16_t colors) : _palette(colors)
{
}

bool Palette::savePAL(std::ostream &output, bool fullpal)
{
    if(fullpal) {
        //full pal currently only used for pcx, needs this marker.
        output.put(0x0C);
        for (uint16_t i = 0; i < _palette.size(); i++){
            output.put(_palette[i].r);
            output.put(_palette[i].g);
            output.put(_palette[i].b);
        }
    } else {
        for (uint16_t i = 0; i < _palette.size(); i++){
            output.put(_palette[i].r>>2);
            output.put(_palette[i].g>>2);
            output.put(_palette[i].b>>2);
        }
    }
    return true;
}

Palette::~Palette()
{
}

uint8_t Palette::nearest(Color a)
{
    uint8_t rv = 0;
    double dist = colorDistance(a, _palette[0]);
    
    for(uint32_t i = 1; i < _palette.size(); i++) {
        double newdist = colorDistance(a, _palette[i]);
        if(newdist < dist) {
            dist = newdist;
            rv = i;
        }
    }
    
    return rv;
}

double Palette::colorDistance(Color cola, Color colb)
{
    long rmean = ( (long)cola.r + (long)colb.r ) / 2;
    long r = (long)cola.r - (long)colb.r;
    long g = (long)cola.g - (long)colb.g;
    long b = (long)cola.b - (long)colb.b;
    return std::sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}

}
