#include "eastwood/TmpFile.h"
#include "eastwood/IStream.h"
#include "eastwood/StdDef.h"

#include "eastwood/Log.h"

#include "eastwood/Exception.h"

/*
0x00 int16 tile_X (always 24)
0x02 int16 tile_Y (always 24)
0x04 int32 frames  // count of tiles in set, not same as images
0x08 int16 frames_x  //0x0000 in td format
0x0A int16 frames_y  //missing in td format
0x0C int32 file_length
0x10 int32 offset_endofheader (always 0x00000028 in ra 0x00000020 in td)
0x14 int32 unkn_00 (seems to always be 0x00000000)
0x18 int32 unkn_01 (unknown always 0xFFFF0D1A in td?)
0x1C int32 offset_footer01 // array of images length, unknown, 
0x20 int32 offset_footer02 // terrain type index, ra only
0x24 int32 offset_footer03  // image index in ra, dword earlier in td
*/

namespace eastwood {

const int16_t TILE_SIZE = 0x18;
const int32_t TD_HEADER_SIZE = 0x20;
const int32_t RA_HEADER_SIZE = 0x28;

BaseImage BLANK(TILE_SIZE, TILE_SIZE);

TmpFile::TmpFile(std::istream& stream)
    : BaseImageSequence(), _unknown(), _terrainindex(), _imageindex(),
      _x(), _y(), _raformat(false)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    
    //first two shorts should be same as TILE_SIZE for valid TMP file
    if(_stream.getU16LE() != TILE_SIZE) {
        throw(Exception(LOG_ERROR, "TmpFile", "Not a valid TMP file"));
    }
    
    if(_stream.getU16LE() != TILE_SIZE) {
        throw(Exception(LOG_ERROR, "TmpFile", "Not a valid TMP file"));
    }
    
    int tilecount = _stream.getU32LE();
    LOG_DEBUG("Tilecount is %d", tilecount);
    _x = _stream.getU16LE();
    
    int filesize;
    if(_x < RA_HEADER_SIZE) {
        _y = _stream.getU16LE();
        _raformat = true;
        filesize = _stream.getU32LE();
    } else {
        filesize = _x + (_stream.getU16LE() << 16);
        _x = tilecount;
        _y = 1;
    }
    
    LOG_DEBUG("filesize %d", filesize);
    
    int imgdataoffset =  _stream.getU32LE();
    
    //skip unknown stuff
    _stream.ignore(8);
    
    //get offsets to footers
    int footer1 = _stream.getU32LE();
    int footer2 = 0;
    if(_raformat) {
        footer2 = _stream.getU32LE();
    }
    int footer3 = _stream.getU32LE();
    
    if(_raformat) {
        _terrainindex.resize(tilecount);
        _stream.seekg(footer2, std::ios_base::beg);
        _stream.read(&_terrainindex.at(0), tilecount);
    }
    
    _imageindex.resize(tilecount);
    _stream.seekg(footer3, std::ios_base::beg);
    _stream.read(&_imageindex.at(0), tilecount);
    
    //read the image data for each tile if valid image index
    _stream.seekg(imgdataoffset, std::ios_base::beg);
    
    for(int i = 0; i < tilecount; i++) {
        if(_imageindex[i] != 0xFF) {
            _frames.push_back(BaseImage(TILE_SIZE, TILE_SIZE));
            _stream.read(reinterpret_cast<char*>(static_cast<uint8_t*>(_frames.at(i))),
                         TILE_SIZE * TILE_SIZE);
        }
    }
}

BaseImage& TmpFile::operator [](uint16_t i)
{
    char imgindex = _imageindex[i];
    return imgindex < 0 ? BLANK : _frames.at(imgindex);
}

Surface TmpFile::getSurface(Palette pal)
{
    Surface surf(_x * TILE_SIZE, _y * TILE_SIZE, 8, pal);
    
    int index = 0;
    for(int y = 0; y < _y; y++) {
        for(int x = 0; x < _x; x++) {
            char imgindex = _imageindex[index++];
            if(imgindex < 0) {
                BLANK.render(surf, x * TILE_SIZE, y * TILE_SIZE);
            } else {
                _frames.at(imgindex).render(surf, x * TILE_SIZE, y * TILE_SIZE);
            }
        }
    }
    
    return surf;
}

} //eastwood
