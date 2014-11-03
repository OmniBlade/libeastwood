#include "eastwood/AudFile.h"
#include "eastwood/IStream.h"
#include "eastwood/codec/adpcm.h"
#include "eastwood/Exception.h"
#include "eastwood/Log.h"

namespace eastwood {

AudFile::AudFile(std::istream& stream) 
    : _sound(), _frequency(0), _compressedsize(0), _size(0), _flags(0), _format(0)
{
    IStream& _stream = reinterpret_cast<IStream&>(stream);
    
    _frequency = _stream.getU16LE();
    _compressedsize = _stream.getU32LE();
    LOG_DEBUG("compressed size has value of %d", _compressedsize);
    _size = _stream.getU32LE();
    _flags = _stream.get();
    _format = _stream.get();
    
    if(_frequency != 22050) {
        LOG_DEBUG("Frequency not correct, has value of %d", _frequency);
        throw(Exception(LOG_ERROR, "AudFile", "Aud is not in the expected format"));
    }
    
    if(_compressedsize + 12 != _stream.sizeg()) {
        LOG_DEBUG("Filesize does not match that reported by the header");
        throw(Exception(LOG_ERROR, "AudFile", "Filesize header mismatch"));
    }
    
    _sound.resize(_compressedsize);
    _stream.read(reinterpret_cast<char*>(&_sound.at(0)), _compressedsize);
}

Sound AudFile::getSound()
{
    uint8_t* buffer = new uint8_t[_size];
    uint8_t channels = _flags & 0x01 ? 2 : 1;
    AudioFormat format = _flags & 0x02 ? FMT_S16LE : FMT_U8;
    
    if(_format == IMA_ADPCM){
        LOG_DEBUG("Decompressing IMA");
        getIMA(buffer);
    } else if (_format == WW_ADPCM) {
        LOG_DEBUG("Decompressing WW");
        getWW(buffer);
    } else {
        throw(Exception(LOG_ERROR, "AudFile", "Compression format not supported"));
    }
    
    LOG_DEBUG("Creating sound object for format %d, channels %d", format, channels);
    //create sound object from decode buffer, object takes ownership
    return Sound(_size, buffer, channels, _frequency, format);
}

void AudFile::getIMA(uint8_t* buffer)
{
    uint8_t* readp = &_sound.at(0);
    uint8_t* writep = buffer;
    uint8_t* endp = readp + _compressedsize;
    
    //initialise values for imaadpcm loop
    int sample = 0;
    int index = 0;
    int decompressed = 0;
    while(readp < endp) {
        //get compressed size then move past chunk header
        uint16_t cmpsize = *readp++;
        cmpsize += *readp++ << 8;
        uint16_t size = *readp++;
        size += *readp++ << 8;
        uint32_t id = *reinterpret_cast<uint32_t*>(readp);
        readp += 4;
        //decompress chunk, decoder will move pointers
        codec::decodeIMA(readp, writep, cmpsize, sample, index);
        readp += cmpsize;
        writep += size;
        decompressed += size;
    }
}

void AudFile::getWW(uint8_t* buffer)
{
    uint8_t* readp = &_sound.at(0);
    uint8_t* writep = buffer;
    uint8_t* endp = readp + _compressedsize;
    
    //initialise values for imaadpcm loop
    int decompressed = 0;
    while(readp < endp) {
        //get compressed size then move past chunk header
        uint16_t cmpsize = *readp++;
        cmpsize += *readp++ << 8;
        uint16_t size = *readp++;
        size += *readp++ << 8;
        uint32_t id = *reinterpret_cast<uint32_t*>(readp);
        readp += 4;
        //decompress chunk, decoder will move pointers
        codec::decodeWWS(readp, writep, cmpsize, size);
        readp += cmpsize;
        writep += size;
        decompressed += size;
    }
}

} //eastwood
