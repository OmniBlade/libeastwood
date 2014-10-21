#ifndef EASTWOOD_CODEC_LCW_H
#define	EASTWOOD_CODEC_LCW_H

#include "../StdDef.h"
#include <istream>
#include <ostream>

namespace eastwood { namespace codec {

    int decodeLCW(const uint8_t* source, uint8_t* dest, int destsize);
    int encodeLcw(const uint8_t* src, uint8_t* dest, int datasize);
    
} } //eastwood codec
    
#endif	/* EASTWOOD_CODEC_LCW_H */
