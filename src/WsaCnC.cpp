#include <vector>
#include <stdexcept>
#include <iostream>

#include "eastwood/StdDef.h"

#include "eastwood/WsaCnC.h"

#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/Decode.h"
#include "eastwood/PalFile.h"

namespace eastwood {

WsaCnC::WsaCnC(CCFileClass& fclass, Surface firstFrame) :
    _width(0), _height(0), _xpos(0), _ypos(0), _frameOffsTable(0), _palette(0),
    _decodedFrames(0), _deltaBufferSize(0), _framesPer1024ms(0)
{
    //uint32_t frameDataOffs;

    _decodedFrames.resize(fclass.readle16());
    LOG_INFO("WsaFile: numframes = %d", _decodedFrames.size());
    
    _xpos = fclass.readle16();
    _ypos = fclass.readle16();
    _width = fclass.readle16();
    _height = fclass.readle16();
    LOG_INFO("WsaFile: size %d x %d", _width, _height);

    _deltaBufferSize = fclass.readle32();

    _frameOffsTable.resize(_decodedFrames.size()+2);
    for (uint32_t i = 0; i < _frameOffsTable.size(); ++i) {
	_frameOffsTable[i] = fclass.readle32();
	if (_frameOffsTable[i])
	    _frameOffsTable[i] += 768; //offset to account for palette;
    }

    _framesPer1024ms = _deltaBufferSize / 1024.0f;

    LOG_INFO("WsaFile", "_framesPer1024ms = %d", _framesPer1024ms);

    PalFile pal(fclass);
    _palette = pal.getPalette();
    _decodedFrames.front() = firstFrame ? firstFrame : Surface(_width, _height, 8, _palette);
    decodeFrames(fclass);
}

WsaCnC::~WsaCnC()
{
}

void WsaCnC::decodeFrames(CCFileClass& fclass)
{
    std::vector<uint8_t> dec80(_decodedFrames.front().size());
    Surface *pic = NULL;

    for(std::vector<Surface>::iterator it = _decodedFrames.begin();
	    it != _decodedFrames.end(); pic = &(*it), ++it) {
	if(pic)
	    *it = Surface(*pic);
	Decode::decode80(fclass, &dec80.front());
	Decode::decode40(&dec80.front(), *it);

    }
}

}

