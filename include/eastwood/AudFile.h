#ifndef EASTWOOD_AUDFILE_H
#define	EASTWOOD_AUDFILE_H

#include "eastwood/Sound.h"
#include "eastwood/StdDef.h"
#include <istream>
#include <vector>

namespace eastwood {

class AudFile
{
public:
    AudFile(std::istream& stream);
    ~AudFile() {};
    
    Sound getSound();
private:
    enum compressionFormat {
        WW_ADPCM  = 1,
        IMA_ADPCM = 99
    };
    void getIMA(uint8_t* buffer);
    void getWW(uint8_t* buffer);
    std::vector<uint8_t> _sound;
    int16_t _frequency;
    int32_t _compressedsize; //without header so filesize - 12
    int32_t _size;
    int8_t  _flags;     //bit 0=stereo, bit 1=16bit
    int8_t  _format;    //1=WW compressed, 99=IMA ADPCM
};

} //eastwood

#endif	/* EASTWOOD_AUDFILE_H */

