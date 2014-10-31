#ifndef EASTWOOD_CODEC_ADPCM_H
#define	EASTWOOD_CODEC_ADPCM_H

#include "../StdDef.h"
#include <istream>
#include <ostream>
#include <vector>

namespace eastwood { namespace codec {

    void decodeIMA(std::istream& src, uint8_t* dest, uint16_t compressed_size, 
                   int& sample, int& index);
    void decodeIMA(uint8_t* src, uint8_t* dest, uint16_t compressed_size, 
                   int& sample, int& index);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_ADPCM_H */

