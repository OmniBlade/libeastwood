#ifndef EASTWOOD_CODEC_ADPCM_H
#define	EASTWOOD_CODEC_ADPCM_H

#include "../StdDef.h"
#include <istream>
#include <ostream>
#include <vector>

namespace eastwood { namespace codec {
    
 /**
  * @brief ADPCM codecs
  * 
  * Codecs for handling audio files compressed with either IMA ADPCM or 
  * Westwood ADPCM compression formats. 
  *      
  */
    
    /**
     * @brief Decode IMA ADPCM from a std::istream
     * @param src istream reference
     * @param dest destination buffer
     * @param cmpsize compressed size of the data to decode
     * @param sample reference to an int, caller maintains value between calls
     * @param index reference to an int, caller maintains value between calls
     */
    void decodeIMA(std::istream& src, uint8_t* dest, uint16_t cmpsize, 
                   int& sample, int& index);
    /**
     * @brief Decode IMA ADPCM from a unsigned char buffer
     * @param src source buffer pointer
     * @param dest destination buffer pointer
     * @param cmpsize compressed size of the data to decode
     * @param sample reference to an int, caller maintains value between calls
     * @param index reference to an int, caller maintains value between calls
     */
    void decodeIMA(uint8_t* src, uint8_t* dest, uint16_t cmpsize, 
                   int& sample, int& index);
    /**
     * @brief Decode Westwood ADPCM from a unsigned char buffer
     * @param src source buffer pointer
     * @param dest destination buffer pointer
     * @param cmpsize compressed size of the data to decode
     * @param size expected size of the uncompressed data
     */
    void decodeWWS(uint8_t* src, uint8_t* dest, uint16_t cmpsize, uint16_t size);
    
} } //eastwood codec

#endif	/* EASTWOOD_CODEC_ADPCM_H */

