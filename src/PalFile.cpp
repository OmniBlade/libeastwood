#include <istream>

#include "eastwood/StdDef.h"
#include "eastwood/Log.h"

#include "eastwood/PalFile.h"

namespace eastwood {

PalFile::PalFile(CCFileClass& fclass) : _palette(256)
{
    for (uint16_t i = 0; i < _palette.size(); i++){
	_palette[i].r = fclass.read8()<<2;
	_palette[i].g = fclass.read8()<<2;
	_palette[i].b = fclass.read8()<<2;
	_palette[i].unused = 0;
    }
}

PalFile::~PalFile()
{
}

Palette PalFile::getPalette()
{
    return _palette;
}

}
