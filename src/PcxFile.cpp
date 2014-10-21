#include "eastwood/PcxFile.h"
#include "eastwood/StdDef.h"
#include "eastwood/Exception.h"
#include "eastwood/PalFile.h"
#include "eastwood/codec/rle.h"


namespace eastwood {

const uint8_t ZSOFT_SIG = 0x0A;    //zsoft pcx format signature
const int OFFSET = 128;         //size of pcx header
const uint16_t DPI = 300;

PcxFile::PcxFile(std::istream &stream):
    BaseImage(), _header(), _palette(0)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    
    _header.sig = _stream.get();
    if(_header.sig != ZSOFT_SIG)
        throw(Exception(LOG_ERROR, "PcxFile", "Not a zSoft PCX"));
    
    _header.ver = _stream.get();
    if(_header.ver != V30_STD)
        throw(Exception(LOG_ERROR, "PcxFile", "Formats other than 5 not handled"));
    
    _header.enc = _stream.get();
    if(_header.enc != 1)
        throw(Exception(LOG_ERROR, "PcxFile", "Encoding byte not correct"));
    
    _header.bpp = _stream.get();
    if(_header.bpp < 8)
        throw(Exception(LOG_ERROR, "PcxFile", "PCX of bit depth < 8 not handled"));
    //_Bpp = _bpp / 8;
    
    _header.xmin = _stream.getU16LE();
    _header.ymin = _stream.getU16LE();
    _header.xmax = _stream.getU16LE();
    _header.ymax = _stream.getU16LE();
    _header.hres = _stream.getU16LE();
    _header.vres = _stream.getU16LE();
    
    //get resolution and 16 color palette
    _stream.read(reinterpret_cast<char*>(_header.pal16), 48);
    _header.res = _stream.get();
    
    //again make sure we have a 256 colour image
    _header.planes = _stream.get();
    if(_header.planes != 1)
        throw(Exception(LOG_ERROR, "PcxFile", "Bitplanes > 1 not supported"));
    
    _header.bpl = _stream.getU16LE();
    _height = (_header.ymax - _header.ymin) + 1;
    _width = _header.bpl;
    //resize out bitmap to match expected
    _bitmap.resize(_width * _height);
    _header.paltype = _stream.getU16LE();
    
    //fetch pal from end of file
    _stream.seekg(_stream.sizeg() - 768, std::ios_base::beg);
    PalFile pal(_stream, true);
    _palette = pal.getPalette();
    
    //decode image from end of header
    _stream.seekg(OFFSET, std::ios_base::beg);
    codec::decodeRLE(_stream, &_bitmap.at(0));
}

void PcxFile::writePcx(std::ostream& stream)
{
    writeHeader(stream);
    codec::encodeRLE(&_bitmap.at(0), stream, _width, _height);
    _palette.savePAL(stream, true);
}

void PcxFile::writeHeader(std::ostream& stream)
{
    OStream& _stream= reinterpret_cast<OStream&>(stream);
    _stream.put(ZSOFT_SIG);
    _stream.put(V30_STD);
    //encoding
    _stream.put(_header.enc);
    //bpp
    _stream.put(_header.bpp);
    _stream.putU16LE(_header.xmin);
    _stream.putU16LE(_header.ymin);
    _stream.putU16LE(_header.xmax);
    _stream.putU16LE(_header.ymax);
    _stream.putU16LE(_header.hres);
    _stream.putU16LE(_header.vres);
    _stream.write(reinterpret_cast<char*>(_header.pal16), 48);
    _stream.put(_header.res);
    _stream.put(_header.planes);
    _stream.putU16LE(_header.bpl);
    _stream.putU16LE(_header.paltype);
    _stream.seekp(58, std::ios_base::cur);
}

Surface PcxFile::getSurface() {
    Surface surf(_width, _height, 8, _palette);
    memcpy(surf, &_bitmap.at(0), _bitmap.size());
    
    return surf;
}

} //eastwood
