#include <istream>

#include "eastwood/StdDef.h"
#include "eastwood/Log.h"

#include "eastwood/PalFile.h"

namespace eastwood {

PalFile::PalFile(std::istream &stream, bool fullpal) : _palette(256)
{
    if(fullpal){
        for (uint16_t i = 0; i < _palette.size(); i++){
            _palette[i].r = stream.get();
            _palette[i].g = stream.get();
            _palette[i].b = stream.get();
            _palette[i].unused = 0;
        }
    } else {
        for (uint16_t i = 0; i < _palette.size(); i++){
            _palette[i].r = stream.get()<<2;
            _palette[i].g = stream.get()<<2;
            _palette[i].b = stream.get()<<2;
            _palette[i].unused = 0;
            //LOG_DEBUG("Red %d, green %d, blue %d", _palette[i].r, _palette[i].g, _palette[i].b);
        }
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
