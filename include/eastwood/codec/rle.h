#ifndef EASTWOOD_CODEC_RLE_H
#define	EASTWOOD_CODEC_RLE_H

#include "../IStream.h"
#include "../OStream.h"
#include "../StdDef.h"

namespace eastwood { namespace codec {

/**
 * @brief RLE codecs
 * 
 * Codecs for handling RLE data as used in the ZSoft PCX image format
 *      
 */
    
    /**
     * @brief Decode RLE compressed data
     * @param src stream reference for input data
     * @param dest pointer to destination buffer
     * @return size of decoded data
     */
    int decodeRLE(std::istream& src, uint8_t* dest);
    /**
     * @brief Decode RLE compressed data
     * @param src pointer to source data buffer
     * @param dest stream reference for output
     * @param x resolution of the image in the x axis
     * @param y resolution of the image in the y axis
     * @return size of encoded data
     */
    int encodeRLE(const uint8_t* src, std::ostream& dest, int _x = 640, int y = 400);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_RLE_H */

