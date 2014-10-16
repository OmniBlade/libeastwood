#ifndef EASTWOOD_CODEC_FORMAT40_H
#define	EASTWOOD_CODEC_FORMAT40_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {
    
    int decode40(const uint8_t* src, uint8_t* dest);
    int decode40(std::istream& src, uint8_t* dest);
    int encode40(const uint8_t* src, uint8_t* base, uint8_t* dest, int len);
    int encode40(const uint8_t* src, uint8_t* base, std::ostream& dest, int len);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_FORMAT40_H */

