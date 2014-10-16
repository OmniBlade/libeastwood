#ifndef EASTWOOD_CODEC_BASE64_H
#define	EASTWOOD_CODEC_BASE64_H

#include "../StdDef.h"
#include <string>

namespace eastwood { namespace codec {
    
    int decodeBase64(std::string src, uint8_t* dest);
    std::string encodeBase64(uint8_t* src, int len);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_BASE64_H */

