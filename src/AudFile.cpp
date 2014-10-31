#include "eastwood/AudFile.h"
#include "eastwood/IStream.h"
#include "eastwood/codec/adpcm.h"
#include "eastwood/Exception.h"

namespace eastwood {

AudFile::AudFile(std::istream& stream) 
{
    IStream& _stream = reinterpret_cast<IStream&>(stream);
    _frequency = _stream.getU16LE();
    _compressedsize = _stream.getU32LE();
    _size = _stream.getU32LE();
    _flags = _stream.get();
    _format = _stream.get();
    
    if(_frequency != 22050) {
        throw(Exception(LOG_ERROR, "AudFile", "Aud is not in the expected format"));
    }
    
    _stream.read(reinterpret_cast<char*>(&_sound.at(0)), _compressedsize);
}

Sound AudFile::getSound()
{
    
}

} //eastwood
