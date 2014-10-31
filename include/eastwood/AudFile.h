#ifndef EASTWOOD_AUDFILE_H
#define	EASTWOOD_AUDFILE_H

#include "eastwood/Sound.h"
#include "eastwood/StdDef.h"
#include <istream>

class AudFile
{
public:
    AudFile(std::istream& stream);
    ~AudFile();
    
    Sound getSound();
private:
    std::vector<uint8_t> _sound;
    int16_t _frequency;
    int32_t _compressedsize; //without header so filesize - 12
    int32_t _size;
    int8_t  _flags;     //bit 0=stereo, bit 1=16bit
    int8_t  _format;    //1=WW compressed, 99=IMA ADPCM
};

#endif	/* EASTWOOD_AUDFILE_H */

