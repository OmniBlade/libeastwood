#include <iostream>
#include <string>
#include <vector>

#include "eastwood/StdDef.h"

#include "eastwood/CpsFile.h"
#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/PalFile.h"
#include "eastwood/codec/format80.h"

namespace eastwood {

CpsFile::CpsFile(std::istream &stream, Palette palette) :
    BaseImage(320, 200, palette), _format(UNCOMPRESSED)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    
    if(static_cast<uint16_t>(_stream.getU16LE() + _stream.gcount()) != _stream.sizeg())
	throw(Exception(LOG_ERROR, "CpsFile", "Invalid file size"));

    _format = static_cast<compressionFormat>(_stream.getU16LE());
    switch(_format) {
	case UNCOMPRESSED:
	case FORMAT_80:
	    break;
	case FORMAT_LBM:
	    throw(Exception(LOG_ERROR, "CpsFile", "LBM format support not implemented"));
	default:
	    char error[256];
	    snprintf(error, sizeof(error), "Format not supported: %x", _format);
	    throw(Exception(LOG_ERROR, "CpsFile", error));
    }

    if(_stream.getU16LE() + _stream.getU16LE() != _width *_height)
	throw(Exception(LOG_ERROR, "CpsFile", "Invalid image size"));

    if(_stream.getU16LE() == 768){
	if(palette)
	    _stream.ignore(768);
	else {
    	    PalFile pal(_stream);
    	    _palette = pal.getPalette();
	}
    }
    else if(!_palette)
	throw(Exception(LOG_ERROR, "CpsFile", "No palette provided as argument or embedded in CPS"));
    
    int checksum;
    switch(_format) {
	case UNCOMPRESSED:
	    _stream.read(reinterpret_cast<char*>(&_bitmap.at(0)), _bitmap.size());
	    break;
	case FORMAT_LBM:
	    //TODO: implement?
	    throw(Exception(LOG_ERROR, "CpsFile", "LBM format not yet supported"));
	    break;
	case FORMAT_80:
            checksum = codec::decode80(_stream, &_bitmap.at(0));
            if(checksum != _bitmap.size()) {
                LOG_ERROR("Decode80 return %d did not match expected size %d", checksum, _bitmap.size());
                throw(Exception(LOG_ERROR, "CpsFile", "Cannot decode Cps-File"));
            }
            break;
	default:
	    throw(Exception(LOG_ERROR, "CpsFile", "Unknown format"));
    }
}

void CpsFile::writeCps(std::ostream& stream)
{
    OStream& _stream= reinterpret_cast<OStream&>(stream);
    //write 0 for size, correct to filesize - 2 later after encode
    _stream.putU16LE(0);
    _stream.putU16LE(_format);
    _stream.putU32LE(_bitmap.size());
    _stream.putU16LE(_palette.size() * 3);
    if(_palette.size() == 256) _palette.savePAL(_stream);
    codec::encode80(&_bitmap.at(0), _stream, _bitmap.size());
    uint16_t fsize = _stream.tellp();
    _stream.seekp(0, std::ios_base::beg);
    _stream.putU16LE(fsize - 2);
    
}

}
