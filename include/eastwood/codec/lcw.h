#ifndef EASTWOOD_CODEC_LCW_H
#define	EASTWOOD_CODEC_LCW_H

#include "../StdDef.h"
#include <istream>
#include <ostream>
#include <vector>

namespace eastwood { namespace codec {
    
/**
 * @brief LCW codecs
 * 
 * Codecs for handling Westwood Studios proprietary image compression format
 * referred to as LCW internally but better known as format80, format40 and 
 * format2 by the modding community.
 *      
 */
    
    /**
     * @brief Decode LCW (format80) compressed data
     * @param source pointer to source data buffer
     * @param dest pointer to destination buffer
     * @return size of decoded data
     */
    int decodeLCW(const uint8_t* source, uint8_t* dest);
    /**
     * @brief Decode LCW (format80) compressed data
     * @param source stream reference to source data
     * @param dest pointer to destination buffer
     * @return size of decoded data
     */
    int decodeLCW(std::istream& stream, uint8_t* dest);
    /**
     * @brief Apply an XOR Delta (format40) to an existing buffer
     * @param source pointer to source buffer
     * @param dest pointer to destination buffer
     */
    void applyXorDelta(const uint8_t* source , uint8_t* dest);
    /**
     * @brief Fills in RLE index 0 pixels (format2)
     * @param source reference to source vector
     * @param dest pointer to destination buffer
     */
    void fillZeros(const std::vector<uint8_t> &in, uint8_t *dest);
    /**
     * @brief Encode LCW (format80) compressed data
     * @param src pointer to source data buffer
     * @param dest pointer to destination buffer
     * @param datasize size of uncompressed data
     * @return size of encoded data
     */
    int encodeLCW(const uint8_t* src, uint8_t* dest, int datasize);
    /**
     * @brief Encode LCW (format80) compressed data
     * @param src pointer to source data buffer
     * @param dest reference to output stream
     * @param datasize size of uncompressed data
     * @return size of encoded data
     */
    int encodeLCW(const uint8_t* src, std::ostream& dest, int datasize);
    /**
     * @brief Create XOR Delta (format40)
     * @param reference pointer to reference buffer that delta will be against
     * @param result pointer to data in state desired after applying delta
     * @param dest pointer to destination buffer
     * @param size of uncompressed reference and result
     * @return size of encoded data
     */
    int createXorDelta(const uint8_t* reference, 
                       const uint8_t* result, uint8_t* dest, int datasize);
    
} } //eastwood codec
    
#endif	/* EASTWOOD_CODEC_LCW_H */
