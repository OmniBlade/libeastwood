#include <cstdarg>
#include "eastwood/StdDef.h"

#include "eastwood/Log.h"

#include "eastwood/Exception.h"
#include "eastwood/CnCShpFile.h"
#include "eastwood/codec/lcw.h"

namespace eastwood {

CnCShpFile::CnCShpFile(std::istream &stream) :
    BaseImageSequence(), _index(1), _width(0), _height(0)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    uint32_t fileSize = _stream.sizeg();
    uint16_t offset = 0;
    uint32_t frames;
    uint32_t tmpint;
    uint32_t lrgframe = 0;
    uint32_t fmt20count = 0;
    uint32_t fmt40count = 0;
    uint32_t fmt80count = 0;

    // First get number of files in shp-file
    frames = _stream.getU16LE();
    LOG_DEBUG("Size read from file was %d", frames);
    
    if(frames == 0)
	throw(Exception(LOG_ERROR, "ShpFile", "There are no frames in this SHP-File!"));


    if(fileSize < static_cast<uint32_t>((frames * 4) + 2 + 2)) {
        LOG_DEBUG("Shp not the expected size");
	throw(Exception(LOG_ERROR, "ShpFile", "SHP file header is incomplete! Header should be %d bytes big, but file is only %d bytes long.",(frames * 4) + 2 + 2, fileSize));
    }
    
    //skip 2 unknown int16
    _stream.ignore(4);
    
    _width = _stream.getU16LE();
    _height = _stream.getU16LE();
    
    //another unknown int32
    _stream.ignore(4);
    
    _index.resize(frames + 2);

    // now fill Index with start and end-offsets
    for(uint16_t i = 0; i < frames + 2; i++) {
        tmpint = _stream.getU32LE();
        _index.at(i).startOffset = tmpint & 0x00FFFFFF;
        _index.at(i).imgFormat = tmpint >> 24;
        tmpint = _stream.getU32LE();
        _index.at(i).refOffset = tmpint & 0x00FFFFFF;
        _index.at(i).refFormat = tmpint >> 24;
        //LOG_DEBUG("Frame %d Offset %d, format %d", i, _index.at(i).startOffset, _index.at(i).imgFormat);
        #ifndef NDEBUG
        switch(_index.at(i).imgFormat){
        case 0x20:
            fmt20count++;
            break;
        case 0x40:
            fmt40count++;
            break;
        case 0x80:
            fmt80count++;
            break;
        default:
            break;
        }
        #endif
    }
    
    LOG_DEBUG("\nFMT20:%d\nFMT40:%d\nFMT80:%d", fmt20count, fmt40count, fmt80count);
    
    for(uint32_t i = 0; i < size; i++) {
        uint8_t format = _index.at(i).imgFormat;
        uint32_t refindex;
        switch(format){
        case 0x80:
            _frames.push_back(BaseImage(_width, _height));
            break;
        case 0x40:
            refindex = getIndex(_index.at(i).refOffset);
            _frames.push_back(_frames.at(refindex));
            break;
        case 0x20:
            _frames.push_back(_frames.at(i - 1)); 
            break;
        }
        decodeFrame(_stream, i, static_cast<uint8_t*>_frames.at(i));
    }
}

CnCShpFile::~CnCShpFile()
{
}

static void apply_pal_offsets(const std::vector<uint8_t> &offsets, uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
	data[i] = offsets[data[i]];
}

#if 0
Surface CnCShpFile::getSurfaceArray(uint8_t tilesX, uint8_t tilesY, ...) {
    std::vector<uint32_t> tiles(tilesX*tilesY);

    va_list arg_ptr;
    va_start(arg_ptr, tilesY);

    for(uint32_t i = 0; i < tilesX*tilesY; i++) {
	tiles[i] = va_arg( arg_ptr, uint32_t );
	if(getIndex(tiles[i]) >= _size)
	    throw(Exception(LOG_ERROR, "ShpFile", "getSurfaceArray(): There exist only %d files in this *.shp.",_size));
    }

    va_end(arg_ptr);
    return getSurfaceArray(tilesX, tilesY, &tiles.front());
}

Surface CnCShpFile::getSurfaceArray(const uint8_t tilesX, const uint8_t tilesY, const uint32_t *tiles) {
    uint8_t width,
	    height;
    uint16_t index = getIndex(tiles[0]);

    _stream.seekg(_index.at(index).startOffset+3, std::ios::beg);
    width = _stream.getU16LE();
    height = _stream.get();    

    for(uint32_t i = 1; i < tilesX*tilesY; i++) {
	_stream.seekg(_index.at(getIndex(tiles[i])).startOffset+2, std::ios::beg);
	if(_stream.get() != height || _stream.get() != width) {
	    throw(Exception(LOG_ERROR, "ShpFile", "getSurfaceArray(): Not all pictures have the same size!"));
	}
    }

    Surface pic(width*tilesX, height*tilesY, 8, _palette);

    for(uint32_t j = 0; j < tilesY; j++)	{
	for(uint32_t i = 0; i < tilesX; i++) {

	    Surface imageOut = getSurface(getIndex(tiles[j*tilesX+i]));

	    //Now we can copy line by line
	    switch(getType(tiles[i])) {
		case TILE_NORMAL:
		    for(int y = 0; y < height; y++)
			memcpy(reinterpret_cast<char*>(static_cast<uint8_t*>(pic)) + i*width + (y+j*height) * pic.pitch(), static_cast<uint8_t*>(imageOut) + y * width, width);
		    break;

		case TILE_FLIPH:
		    for(int y = 0; y < height; y++)
			memcpy(reinterpret_cast<char*>(static_cast<uint8_t*>(pic)) + i*width + (y+j*height) * pic.pitch(), static_cast<uint8_t*>(imageOut) + (height-1-y) * width, width);
		    break;

		case TILE_FLIPV:
		    for(int y = 0; y < height; y++)
			for(int x = 0; x < width; x++)
			    *(reinterpret_cast<char*>(static_cast<uint8_t*>(pic)) + i*width + (y+j*height) * pic.pitch() + x) = *(static_cast<uint8_t*>(imageOut) + y * width + (width-1-x));
		    break;

		case TILE_ROTATE:
		    for(int y = 0; y < height; y++)
			for(int x = 0; x < width; x++)
			    *(reinterpret_cast<char*>(static_cast<uint8_t*>(pic)) + i*width + (y+j*height) * pic.pitch() + x) = *(static_cast<uint8_t*>(imageOut) + (height-1-y) * width + (width-1-x));
		    break;

		default:
		    throw(Exception(LOG_ERROR, "ShpFile", "Invalid type for this parameter. Must be one of TILE_NORMAL, TILE_FLIPH, TILE_FLIPV or TILE_ROTATE!"));
		    break;
	    }
	}
    }

    return pic;
}
#endif

void CnCShpFile::decodeFrame(std::istream &stream, uint16_t fileIndex, uint8_t* imageOut)
{
    if (fileIndex >= _index.size() - 2)
    {
        throw(Exception(LOG_ERROR, "ShpFile", "Requested frame out of range"));
    }
    
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    
    LOG_DEBUG("Frame %d Offset %d, format %d", fileIndex, _index.at(fileIndex).startOffset, _index.at(fileIndex).imgFormat);
    
    _stream.seekg(_index.at(fileIndex).startOffset, std::ios_base::beg);
    
    uint32_t len;
    std::vector<uint8_t> source;
    
    switch (_index.at(fileIndex).imgFormat) {
        case 0x80:
            codec::decodeLCW(_stream, imageOut);
            break;
        case 0x40:{
            uint32_t refimage = getIndex(_index.at(fileIndex).refOffset);
            len = _index.at(fileIndex + 1).startOffset - _index.at(fileIndex).startOffset;
            source.resize(len);
            source.clear();
            _stream.read(reinterpret_cast<char*>(&source.front()), len);
            codec::applyXorDelta(reinterpret_cast<uint8_t*>(&source.front()), imageOut);
            break;
        }
        case 0x20:
            len = _index.at(fileIndex + 1).startOffset - _index.at(fileIndex).startOffset;
            source.resize(len);
            source.clear();
            _stream.read(reinterpret_cast<char*>(&source.front()), len);
            codec::applyXorDelta(reinterpret_cast<uint8_t*>(&source.front()), imageOut);
            break;
        default:
            throw(Exception(LOG_ERROR, "ShpFile", "Image format not recognized"));
    }
    //return Surface(imageOut, _width, _height, 8, _palette);
}

uint32_t CnCShpFile::getIndex(uint32_t offset)
{
    for(uint32_t i = 0; i < _index.size(); i++) {
        if(offset == _index.at(i).startOffset){
            return i;
        }
    }
    throw(Exception(LOG_ERROR, "ShpFile", "A required offset not indexed"));
}

}
