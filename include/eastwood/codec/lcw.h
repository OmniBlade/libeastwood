#ifndef EASTWOOD_CODEC_LCW_H
#define	EASTWOOD_CODEC_LCW_H

#include "../StdDef.h"
#include <istream>
#include <ostream>

namespace eastwood { namespace codec {

    int decodeLCW(const uint8_t* source, uint8_t* dest, int destsize);
    int encodeLCW(const uint8_t* src, uint8_t* dest, int datasize);
    int encodeLCW(const uint8_t* src, std::ostream& dest, int len);
    
} } //eastwood codec
    
#endif	/* EASTWOOD_CODEC_LCW_H */
