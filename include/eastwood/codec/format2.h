#ifndef EASTWOOD_CODEC_FORMAT2_H
#define	EASTWOOD_CODEC_FORMAT2_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {

    int decode2(std::istream& src, uint8_t *dest, int len);
    int encode2(uint8_t* src, std::ostream& dest, int len);

} } //eastwood codec
        
#endif	/* EASTWOOD_CODEC_FORMAT2_H */
