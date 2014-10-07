#include <cstdarg>
#include "eastwood/StdDef.h"

#include "eastwood/Decode.h"

#include "eastwood/Exception.h"
#include "eastwood/ShpCnC.h"

namespace eastwood {

using namespace Decode;

ShpCnC::ShpCnC(CCFileClass& fclass, Palette palette): _palette(palette), _index(1),
_decodedFrames(0), _size(0)
{
    readIndex(fclass);
    LOG_DEBUG("Read index successfully.");
    for(uint32_t i = 0; i < _index.size(); i++) {
        _decodedFrames.push_back(decodeFrame(fclass, i));
    }
}

ShpCnC::~ShpCnC()
{
}

void ShpCnC::readIndex(CCFileClass& fclass)
{
    uint32_t fileSize = fclass.getSize();
    
    // First get number of files in shp-file
    _size = fclass.readle16();
    LOG_DEBUG("Shp contains %d images.", _size);
    
    if(_size == 0)
	throw(Exception(LOG_ERROR, "ShpFile", "There are no files in this SHP-File!"));


    if(fileSize < static_cast<uint32_t>((_size * 4) + 2 + 2)) 
	throw(Exception(LOG_ERROR, "ShpFile", "SHP file header is incomplete! Header should be %d bytes big, but file is only %d bytes long.",(_size * 4) + 2 + 2, fileSize));
    
    //seek past unknown 4 bytes
    fclass.seek(4, SEEK_CUR);
    
    _width = fclass.readle16();
    _height = fclass.readle16();
    
    //another unknown?
    fclass.seek(4, SEEK_CUR);
    
    _index.resize(_size + 2);
    
    for(uint32_t i = 0; i < _size + 2; i++){
        uint32_t tmp = fclass.readle32();
        _index[i].Offset = tmp & 0x00FFFFFF;
        _index[i].Format = tmp >> 24;
        tmp = fclass.readle32();
        _index[i].RefOffs = tmp & 0x00FFFFFF;
        _index[i].RefFormat = tmp >> 24;
    }
}

}//eastwood
