#ifndef EASTWOOD_CODEC_FORMAT80_H
#define	EASTWOOD_CODEC_FORMAT80_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {

    int decode80(uint8_t* src, uint8_t* dest);
    int decode80(std::istream& src, uint8_t* dest);
    int encode80(uint8_t* src, uint8_t* dest, int len);
    int encode80(uint8_t* src, std::ostream& dest, int len);
    
} } //eastwood codec
#endif	/* EASTWOOD_CODEC_FORMAT80_H */

