/* 
 * File:   decode.h
 * Author: aidan
 *
 * Created on 21 September 2014, 23:11
 */

#ifndef EASTWOOD_DECODE_H
#define	EASTWOOD_DECODE_H

#include "StdDef.h"
#include "CnCFileClass.h"

namespace eastwood { namespace Decode {
    /** Decompress format 80 compressed data. */
	int decode80buffer(uint8_t *source, uint8_t *dest, uint16_t destLength);
    int decode80(CCFileClass& fclass, uint8_t *image_out);
	/** Decompress format 40 compressed data. */
	int decode40(const uint8_t* src, uint8_t* dest);
	/** Decompress format 20 compressed data. */
	int decode20(const uint8_t* src, uint8_t* dest, int size);
	/** Decodes base64 data */
	int decodeBase64(const uint8_t* src, uint8_t* dest, const uint32_t length);

} } //eastwood

#endif	/* EASTWOOD_DECODE_H */

