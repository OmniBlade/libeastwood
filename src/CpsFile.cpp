#include <iostream>
#include <string>
#include <vector>

#include "eastwood/StdDef.h"
#include "eastwood/Decode.h"
#include "eastwood/CpsFile.h"
#include "eastwood/Exception.h"
#include "eastwood/Log.h"
#include "eastwood/PalFile.h"

namespace eastwood {

CpsFile::CpsFile(CCFileClass& fclass, Palette palette) :
    Surface(320, 200, 8, palette), _format(UNCOMPRESSED)
{
    LOG_DEBUG("Reading CPS header");
    if(fclass.readle16() + 2 != fclass.getSize())
        throw(Exception(LOG_ERROR, "CpsFile", "Invalid file size"));

    _format = static_cast<compressionFormat>(fclass.readle16());
    switch(_format) {
	case UNCOMPRESSED:
	case FORMAT_80:
            LOG_DEBUG("Format supported");
	    break;
	case FORMAT_LBM:
	    throw(Exception(LOG_ERROR, "CpsFile", "LBM format support not implemented"));
	default:
	    char error[256];
	    snprintf(error, sizeof(error), "Format not supported: %x", _format);
	    throw(Exception(LOG_ERROR, "CpsFile", error));
    }

    if(fclass.readle16() != _width*_height)
	throw(Exception(LOG_ERROR, "CpsFile", "Invalid image size"));
    
    LOG_DEBUG("Validated uncompressed CPS size");

    if(fclass.readle32() == 0x03000000){
	LOG_INFO("CpsFile %s", "CPS has embedded palette");
        PalFile pal(fclass);
        _palette = pal.getPalette();
    }
    else if(!_palette)
	throw(Exception(LOG_ERROR, "CpsFile", "No palette provided as argument or embedded in CPS"));

    switch(_format) {
	case UNCOMPRESSED:
	    fclass.read(reinterpret_cast<char*>(_pixels.get()), size());
	    break;
	case FORMAT_LBM:
	    //TODO: implement?
	    throw(Exception(LOG_ERROR, "CpsFile", "LBM format not yet supported"));
	    break;
	case FORMAT_80:
    	if(Decode::decode80(fclass, reinterpret_cast<uint8_t*>(_pixels.get()), 0) == -2)
    	    throw(Exception(LOG_ERROR, "CpsFile", "Cannot decode Cps-File"));
	break;
	default:
	    throw(Exception(LOG_ERROR, "CpsFile", "Unknown format"));
    }
}

CpsFile::~CpsFile()
{	
}

}
