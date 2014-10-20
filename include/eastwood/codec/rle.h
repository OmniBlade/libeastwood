#ifndef EASTWOOD_CODEC_RLE_H
#define	EASTWOOD_CODEC_RLE_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {

    int decodeRLE(std::istream& src, uint8_t* dest);
    int encodeRLE(const uint8_t* src, std::ostream& dest, int _x = 640, int y = 400);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_RLE_H */

