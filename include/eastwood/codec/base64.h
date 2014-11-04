#ifndef EASTWOOD_CODEC_BASE64_H
#define	EASTWOOD_CODEC_BASE64_H

#include "../StdDef.h"
#include <string>

namespace eastwood { namespace codec {

/**
  * @brief Base64 codecs
  * 
  * Codecs to handle data in Base64 format
  *      
  */
    
    /**
     * @brief Decode string containing Base64 encoded information
     * @param src string containing the data
     * @param dest destination buffer
     * @return size of the uncompressed data
     */
    int decodeBase64(std::string src, uint8_t* dest);
    /**
     * @brief Encode string containing Base64 encoded information
     * @param src pointer to the source buffer
     * @param len length of the uncompressed data
     * @return string containing data in Base64
     */
    std::string encodeBase64(uint8_t* src, int len);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_BASE64_H */

