#include <vector>
#include <stdexcept>

#include "eastwood/StdDef.h"

#include "eastwood/WsaFile.h"
#include "eastwood/PalFile.h"
#include "eastwood/IStream.h"

#include "eastwood/Exception.h"
#include "eastwood/Log.h"

#include "eastwood/codec/lcw.h"
#include "eastwood/codec/format40.h"

namespace eastwood {

WsaFile::WsaFile(std::istream &stream, Palette palette) :
    BaseImageSequence(palette), _frameOffsTable(0),
    _width(0), _height(0), _deltaBufferSize(0), _framesPer1024ms(0)
{
    uint32_t frameDataOffs = 0;
    uint32_t frameCount = 0;
    bool newformat;
    
    IStream& _stream= reinterpret_cast<IStream&>(stream);

    frameCount = _stream.getU16LE();
    LOG_DEBUG("WsaFile numframes = %d", frameCount);

    _width = _stream.getU16LE();
    _height = _stream.getU16LE();
    
    //These shorts will be 0 if a new format wsa for x and y pos
    if(!_width && !_height){
        //following shorts are actual dimensions
        _width = _stream.getU16LE();
        _height = _stream.getU16LE();
        newformat = true;
    }
    
    LOG_DEBUG("WsaFile size %d x %d", _width, _height);
    
    if(newformat){
        _deltaBufferSize = _stream.getU32LE();
        
        _frameOffsTable.resize(frameCount + 2);
        for (uint32_t i = 0; i < _frameOffsTable.size(); ++i) {
            _frameOffsTable[i] = _stream.getU32LE();
            if (_frameOffsTable[i])
                _frameOffsTable[i] += 768; //offset to account for palette;
        }
        
    } else {
        
        _deltaBufferSize = _stream.getU16LE();
        
        // "Regular" WSA files shipped with the Dune 2 demo version does not have
        // 2 bytes padding here...
        if(_stream.getU16LE())
            _stream.seekg(-2, std::ios::cur);

        frameDataOffs = _stream.getU16LE();
        // "Continue" WSA files shipped with the Dune 2 demo version does not have
        // 2 bytes padding here...
        if(_stream.getU16LE())
            _stream.seekg(-2, std::ios::cur);
        
        if (frameDataOffs == 0) {
            frameDataOffs = _stream.getU32LE();
            _frames.pop_back();
        }
        
        _frameOffsTable.resize(frameCount + 2);
        for (uint32_t i = 1; i < _frameOffsTable.size(); ++i) {
            _frameOffsTable[i] = _stream.getU32LE();
            if (_frameOffsTable[i])
                _frameOffsTable[i] -= frameDataOffs;
        }
    }
    
    _framesPer1024ms = _deltaBufferSize / 1024.0f;

    LOG_DEBUG("WsaFile _framesPer1024ms = %d", _framesPer1024ms);
    
    if(newformat){
        PalFile pal(_stream);
        _palette = pal.getPalette();
    }

    decodeFrames(_stream);
}

WsaFile::~WsaFile()
{
}

void WsaFile::decodeFrames(std::istream& stream)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    
    //frame count is 2 less than offset table size;
    uint32_t frameCount = _frameOffsTable.size() - 2;
    std::vector<uint8_t> dec80buf(_height * _width);
    
    //push an empty frame to decoded frames
    _frames.push_back(BaseImage(_width, _height));
    
    LOG_DEBUG("First frame pushed back");
    
    for(uint32_t i = 0; i < frameCount; i++) {
        //make sure our decode buffer is clear then read into it
        dec80buf.clear();
        
        //decode the data into another buffer
        codec::decodeLCW(_stream, &dec80buf.front());
        
        //if this isn't the first frame, we need to copy the last one
        if(i > 0) {
            _frames.push_back(BaseImage(_frames.back()));
        }
        
        //Apply uncompressed delta over copy of the last frame to get current
        codec::applyXorDelta(&dec80buf.front(), _frames.back());
    }
}

/*
Surface WsaFile::getSurface(uint16_t frameNumber)
{
    Surface surf(_width, _height, 8, _palette);
    if(frameNumber >= _frames.size()) {
        throw(Exception(LOG_ERROR, "WsaFile", "Tried to access none existent frame."));
    }
    memcpy(surf, static_cast<void*>(_frames[frameNumber]), _height * _width);
    
    return surf;
}
*/

}
