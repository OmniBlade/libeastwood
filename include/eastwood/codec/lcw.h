#ifndef EASTWOOD_CODEC_LCW_H
#define	EASTWOOD_CODEC_LCW_H

#include "../StdDef.h"
#include <istream>
#include <ostream>
#include <vector>

namespace eastwood { namespace codec {

    int decodeLCW(const uint8_t* source, uint8_t* dest);
    int decodeLCW(std::istream& stream, uint8_t* dest);
    void applyXorDelta(const uint8_t* source , uint8_t* dest);
    void fillZeros(const std::vector<uint8_t> &in, uint8_t *dest);
    int encodeLCW(const uint8_t* src, uint8_t* dest, int datasize);
    int encodeLCW(const uint8_t* src, std::ostream& dest, int datasize);
    int createXorDelta(const uint8_t* reference, 
                       const uint8_t* result, uint8_t* dest, int datasize);
    
} } //eastwood codec
    
#endif	/* EASTWOOD_CODEC_LCW_H */
