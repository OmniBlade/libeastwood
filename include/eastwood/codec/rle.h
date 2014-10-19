#ifndef EASTWOOD_CODEC_RLE_H
#define	EASTWOOD_CODEC_RLE_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {

    int decodeRLE(std::istream& src, uint8_t* dest);
    int encodeRLE(uint8_t* src, std::ostream& dest, int len);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_RLE_H */

