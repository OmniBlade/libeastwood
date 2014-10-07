#include <istream>

#include "eastwood/StdDef.h"
#include "eastwood/Log.h"

#include "eastwood/PalFile.h"

namespace eastwood {

PalFile::PalFile(CCFileClass& fclass, bool ww) : _palette(256)
{
    if(ww) {
        LOG_DEBUG("Getting WW format pal");
        for (uint16_t i = 0; i < _palette.size(); i++){
            _palette[i].r = fclass.read8()<<2;
            _palette[i].g = fclass.read8()<<2;
            _palette[i].b = fclass.read8()<<2;
            _palette[i].unused = 0;
        }
    } else {
        LOG_DEBUG("Getting normal format pal");
        for (uint16_t i = 0; i < _palette.size(); i++){
	_palette[i].r = fclass.read8();
	_palette[i].g = fclass.read8();
	_palette[i].b = fclass.read8();
	_palette[i].unused = 0;
        }
    }
    LOG_DEBUG("Palette successfully read with size %d", _palette.size());
}

PalFile::~PalFile()
{
}

Palette PalFile::getPalette()
{
    return _palette;
}

}
