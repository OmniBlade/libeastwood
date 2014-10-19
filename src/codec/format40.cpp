#include "eastwood/codec/format40.h"

namespace eastwood { namespace codec {


// Small skip command
const uint8_t CMD_SKIP_S = 0x80;	// 10000000
const int CMD_SKIP_S_MAX = 63;	// 01111111, 0x7f
const int CMD_SKIP_S_THRESHOLD = 0;

// Large skip command
const uint8_t CMD_SKIP_L1 = 0x80;	// 10000000
const uint16_t CMD_SKIP_L2 = 0x0000;	// 00000000 00000000
const int CMD_SKIP_L_MAX = 32767;	// 01111111 11111111, 0x7fff

// Small fill command
const uint8_t CMD_FILL_S = 0x00;	// 00000000
const int CMD_FILL_S_MAX = 255;	// 11111111, 0xff
const int CMD_FILL_S_THRESHOLD = 2;

// Large fill command
const uint8_t CMD_FILL_L1 = 0x80;	// 10000000
const uint16_t CMD_FILL_L2 = 0xc000;	// 11000000 00000000
const int CMD_FILL_L_MAX = 16383;	// 00111111 11111111, 0x3fff

// Small XOR command
const uint8_t CMD_XOR_S = 0x00;	// 00000000
const uint8_t CMD_XOR_S_MAX = 63;	// 01111111, 0x7f

// Large XOR command
const uint8_t CMD_XOR_L1 = 0x80;	// 10000000
const uint16_t CMD_XOR_L2 = 0x8000;	// 10000000 00000000
const int CMD_XOR_L_MAX = 16383;	// 00111111 11111111, 0x3fff

static inline void writeLE16(int16_t val, uint8_t*& writep)
{
    val = htole16(val);
    *writep++ = val & 0xff;
    *writep++ = val >> 8;
}

int skipCandidate(const uint8_t* src, const uint8_t* base, int len)
{
    int soffset = 0;
    int boffset = 0;
    
    //find common bytes to skip
    int candidatelen;
    while(soffset < len && candidatelen < CMD_SKIP_L_MAX) {
        if(src[soffset++] != base[boffset++]) {
            break;
        }
        candidatelen++;
    }
    
    return candidatelen > CMD_SKIP_S_THRESHOLD ? candidatelen : 0;
}

int fillCandidate(const uint8_t* src, const uint8_t* base, int len)
{
    int soffset = 0;
    int boffset = 0;
    
    int candidatelen = 1;
    uint8_t srcbyte = src[soffset++];
    uint8_t basebyte = base[boffset++];
    
    while(soffset < len && candidatelen < CMD_FILL_L_MAX) {
        if(src[soffset++] != srcbyte || base[boffset++] != basebyte) {
            break;
        }
        candidatelen++;
    }
    
    return candidatelen > CMD_FILL_S_THRESHOLD ? candidatelen : 0;
}

int xorCandidate(const uint8_t* src, const uint8_t* base, int len)
{
    int soffset = 0;
    int boffset = 0;
    
    int candidatelen = 1;
    int runlen = 1;
    uint8_t lastsrcbyte = src[soffset++];
    uint8_t lastbasebyte = base[boffset++];
    
    while(soffset < len && candidatelen < CMD_XOR_L_MAX) {
        uint8_t nextsrcbyte = src[soffset++];
        uint8_t nextbasebyte = base[boffset++];
        
        if(nextsrcbyte == lastsrcbyte && nextbasebyte == lastbasebyte) {
            runlen++;
            if(runlen == 2) {
                candidatelen -= runlen - 2;
                break;
            }
        } else {
            runlen = 1;
        }
        candidatelen++;
        lastsrcbyte = nextsrcbyte;
        lastbasebyte = nextbasebyte;
    }
    
    return candidatelen;
}

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
    int soffset = 0;
    int boffset = 0;
    int doffset = 0;
    
    while(soffset < len) {
        int skiplen = skipCandidate(src + soffset, base + boffset, len - soffset);
        int filllen = fillCandidate(src + soffset, base + boffset, len - soffset);
        int xorlen = xorCandidate(src + soffset, base + boffset, len - soffset);
        
        int bestlen = std::max(skiplen, std::max(filllen, xorlen));
        
        //skip commands
        if(bestlen == skiplen) {
            //method 6 small skip
            if(skiplen <= CMD_SKIP_S_MAX) {
                dest[doffset++] = CMD_SKIP_S | skiplen;
                
            //method 3 large skip
            } else {
                dest[doffset++] = CMD_SKIP_L1;
                uint16_t leskiplen = htole16(CMD_SKIP_L2 | skiplen);
                memcpy(dest + doffset, reinterpret_cast<uint8_t*>(&leskiplen), 2);
                doffset += 2;
            }
            
            soffset += skiplen;
            boffset += skiplen;
        //fill commands
        } else if(bestlen == filllen) {
            uint8_t xorfillval = src[soffset++] ^ base[boffset++];
            
            //method 1 small xor fill
            if(filllen <= CMD_FILL_S_MAX) {
                dest[doffset++] = CMD_FILL_S;
                dest[doffset++] = filllen;
                
            //method 5 large xor fill
            } else {
                dest[doffset++] = CMD_FILL_L1;
                uint16_t lefilllen = htole16(CMD_FILL_L2 | filllen);
                memcpy(dest + doffset, reinterpret_cast<uint8_t*>(&lefilllen), 2);
                doffset += 2;
            }
            
            dest[doffset++] = xorfillval;
            soffset += filllen - 1;
            boffset += filllen - 1;
        //xor commands
        } else {
            //method 2 small xor
            if(xorlen <= CMD_XOR_S_MAX) {
                dest[doffset++] = CMD_XOR_S | xorlen;
            //method 4 large xor
            } else {
                dest[doffset++] = CMD_XOR_L1;
                uint16_t lexorlen = htole16(CMD_XOR_L2 | xorlen);
                memcpy(dest + doffset, reinterpret_cast<uint8_t*>(&lexorlen), 2);
                doffset += 2;
            }
            
            while(xorlen--) {
                dest[doffset++] = src[soffset++] ^ base[boffset++];
            }
        }
    }
    
    return doffset;
}

int encode40(const uint8_t* src, uint8_t* base, std::ostream& dest, int len)
{
    OStream& _stream = reinterpret_cast<OStream&>(dest);
    int soffset = 0;
    int boffset = 0;
    
    while(soffset < len) {
        int skiplen = skipCandidate(src + soffset, base + boffset, len - soffset);
        int filllen = fillCandidate(src + soffset, base + boffset, len - soffset);
        int xorlen = xorCandidate(src + soffset, base + boffset, len - soffset);
        
        int bestlen = std::max(skiplen, std::max(filllen, xorlen));
        
        //skip commands
        if(bestlen == skiplen) {
            //method 6 small skip
            if(skiplen <= CMD_SKIP_S_MAX) {
                _stream.put(CMD_SKIP_S | skiplen);
                
            //method 3 large skip
            } else {
                _stream.put(CMD_SKIP_L1);
                _stream.putU16LE(CMD_SKIP_L2 | skiplen);
            }
            
            soffset += skiplen;
            boffset += skiplen;
        //fill commands
        } else if(bestlen == filllen) {
            uint8_t xorfillval = src[soffset++] ^ base[boffset++];
            
            //method 1 small xor fill
            if(filllen <= CMD_FILL_S_MAX) {
                _stream.put(CMD_FILL_S);
                _stream.put(filllen);
                
            //method 5 large xor fill
            } else {
                _stream.put(CMD_FILL_L1);
                _stream.putU16LE(CMD_FILL_L2 | filllen);
            }
            
            _stream.put(xorfillval);
            soffset += filllen - 1;
            boffset += filllen - 1;
        //xor commands
        } else {
            //method 2 small xor
            if(xorlen <= CMD_XOR_S_MAX) {
                _stream.put(CMD_XOR_S | xorlen);
                
            //method 4 large xor
            } else {
                _stream.put(CMD_XOR_L1);
                _stream.putU16LE(CMD_XOR_L2 | xorlen);
            }
            
            while(xorlen--) {
                _stream.put(src[soffset++] ^ base[boffset++]);
            }
        }
    }
    
    return _stream.tellp();
}

} } //eastwood codec
