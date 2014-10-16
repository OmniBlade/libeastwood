#include "eastwood/codec/format40.h"

namespace eastwood { namespace codec {


// Small skip command
const uint8_t CMD_SKIP_S = 0x80;	// 10000000
const int CMD_SKIP_S_MAX = 63;	// 01111111, 0x7f
const int CMD_SKIP_S_THRESHOLD = 0;

// Large skip command
const uint8_t CMD_SKIP_L1 = 0x80;	// 10000000
const int16_t CMD_SKIP_L2 = 0x0000;	// 00000000 00000000
const int CMD_SKIP_L_MAX = 32767;	// 01111111 11111111, 0x7fff

// Small fill command
const uint8_t CMD_FILL_S = 0x00;	// 00000000
const int CMD_FILL_S_MAX = 255;	// 11111111, 0xff
const int CMD_FILL_S_THRESHOLD = 2;

// Large fill command
const uint8_t CMD_FILL_L1 = 0x80;	// 10000000
const int16_t CMD_FILL_L2 = 0xc000;	// 11000000 00000000
const int CMD_FILL_L_MAX = 16383;	// 00111111 11111111, 0x3fff

// Small XOR command
const uint8_t CMD_XOR_S = 0x00;	// 00000000
const uint8_t CMD_XOR_S_MAX = 63;	// 01111111, 0x7f

// Large XOR command
const uint8_t CMD_XOR_L1 = 0x80;	// 10000000
const int16_t CMD_XOR_L2 = 0x8000;	// 10000000 00000000
const int CMD_XOR_L_MAX = 16383;	// 00111111 11111111, 0x3fff

int decode40(const uint8_t *src, uint8_t *dest)
{
    /*
       0 fill 00000000 c v
       1 copy 0ccccccc
       2 skip 10000000 c 0ccccccc
       3 copy 10000000 c 10cccccc
       4 fill 10000000 c 11cccccc v
       5 skip 1ccccccc	
       */

    const uint8_t* readp = src;
    uint8_t* writep = dest;
    uint16_t code;
    uint16_t count;
    while (1) {
	code = *readp++;
	if (~code & 0x80) {
	    //bit 7 = 0
	    if (!code) {
		//command 0 (00000000 c v): fill
		count = *readp++;
		code = *readp++;
		while (count--)
		    *writep++ ^= code;
	    } else {
		//command 1 (0ccccccc): copy
		count = code;
		while (count--)
		    *writep++ ^= *readp++;
	    }
	} else {
	    //bit 7 = 1
	    if (!(count = code & 0x7f)) {
		count =  htole16(*(reinterpret_cast<const uint16_t*>(readp)));
		readp += 2;
		code = count >> 8;
		if (~code & 0x80) {
		    //bit 7 = 0
		    //command 2 (10000000 c 0ccccccc): skip
		    if (!count)
			// end of image
			break;
		    writep += count;
		} else {
		    //bit 7 = 1
		    count &= 0x3fff;
		    if (~code & 0x40) {
			//bit 6 = 0
			//command 3 (10000000 c 10cccccc): copy
			while (count--)
			    *writep++ ^= *readp++;
		    } else {
			//bit 6 = 1
			//command 4 (10000000 c 11cccccc v): fill
			code = *readp++;
			while (count--)
			    *writep++ ^= code;
		    }
		}
	    } else {
		//command 5 (1ccccccc): skip
		writep += count;
	    }
	}
    }
    return (writep - dest);
}

int decode40(std::istream& src, uint8_t* dest)
{
    /*
       0 fill 00000000 c v
       1 copy 0ccccccc
       2 skip 10000000 c 0ccccccc
       3 copy 10000000 c 10cccccc
       4 fill 10000000 c 11cccccc v
       5 skip 1ccccccc	
       */
    IStream& _stream= reinterpret_cast<IStream&>(src);

    //const uint8_t* readp = src;
    uint8_t* writep = dest;
    uint16_t code;
    uint16_t count;
    
    while (1) {
	code = _stream.get();
	if (~code & 0x80) {
	    //bit 7 = 0
	    if (!code) {
		//command 0 (00000000 c v): fill
		count = _stream.get();
		code = _stream.get();
		while (count--)
		    *writep++ ^= code;
	    } else {
		//command 1 (0ccccccc): copy
		count = code;
		while (count--)
		    *writep++ ^= _stream.get();
	    }
	} else {
	    //bit 7 = 1
	    if (!(count = code & 0x7f)) {
		count =  _stream.getU16LE();
		code = count >> 8;
		if (~code & 0x80) {
		    //bit 7 = 0
		    //command 2 (10000000 c 0ccccccc): skip
		    if (!count)
			// end of image
			break;
		    writep += count;
		} else {
		    //bit 7 = 1
		    count &= 0x3fff;
		    if (~code & 0x40) {
			//bit 6 = 0
			//command 3 (10000000 c 10cccccc): copy
			while (count--)
			    *writep++ ^= _stream.get();
		    } else {
			//bit 6 = 1
			//command 4 (10000000 c 11cccccc v): fill
			code = _stream.get();
			while (count--)
			    *writep++ ^= code;
		    }
		}
	    } else {
		//command 5 (1ccccccc): skip
		writep += count;
	    }
	}
    }
    return (writep - dest);
}

int encode40(const uint8_t* src, uint8_t* base, uint8_t* dest, int len)
{
    return 0;
}

int encode40(const uint8_t* src, uint8_t* base, std::ostream& dest, int len)
{
    return 0;
}

} } //eastwood codec
