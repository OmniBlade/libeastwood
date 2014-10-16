#include "eastwood/codec/format80.h"
#include "eastwood/Exception.h"

#include <algorithm>

namespace eastwood { namespace codec {

// Transfer command
const uint8_t CMD_TRANSFER = 0x80;         // 10000000
const int     CMD_TRANSFER_MAX = 63;	// 00111111, 0x3f
// Offset copy command
const uint8_t CMD_OFFSET = 0x00;           // 00000000
const int     CMD_OFFSET_MAX = 10;         // -3 = 111, 0x07
const int     CMD_OFFSET_THRESHOLD = 2;	// Must encode at least 3 bytes
const int     CMD_OFFSET_RANGE = 4095;	// 00001111 11111111, 0x0fff
// Small copy command
const uint8_t CMD_COPY_S = 0xc0;           // 11000000
const int     CMD_COPY_S_MAX = 64;         // -3 = 00111101, 0x3d
const int     CMD_COPY_S_THRESHOLD = 2;
// Large copy command
const uint8_t CMD_COPY_L = 0xff;           // 11111111
const int     CMD_COPY_L_MAX = 65535;	// 11111111 11111111, 0xffff
// private static final int CMD_COPY_L_THRESHOLD = 4;
// Colour command
const uint8_t CMD_FILL = 0xfe;             // 11111110
const int     CMD_FILL_MAX = 65535;	// 11111111 11111111, 0xffff
const int     CMD_FILL_THRESHOLD = 3;

static inline void my_memcpy(uint8_t *dst, const uint8_t *src, uint16_t cnt)
{
    /* Copies memory areas that may overlap command by command from small memory
     * addresses to big memory addresses. Thus, already copied commands can be
     * copied again. */
    if (dst + cnt < src || src + cnt < dst) {
	memcpy(dst, src, cnt);
	return;
    }
    for(uint16_t i = 0; i < cnt; i++)
	dst[i] = src[i];
}

int offsetCopyCandidate(const uint8_t* src, int srcoffset, int len)
{
    int rv;
    int coffset = srcoffset;
    //readp
    //offset to start of previous bytes within range
    int doffset = coffset > CMD_OFFSET_RANGE ? CMD_OFFSET_RANGE : 0;
    //donep
    
    int16_t candidatelen = 0;
    int16_t candidatepos = -1;
    
    int pos = doffset;
    while(coffset < len && doffset < srcoffset) {
        //look for a match
        int runlen = 0;
        while(coffset < len && doffset < srcoffset && runlen < CMD_OFFSET_MAX) {
            if(src[coffset++] == src[doffset++]) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        coffset = srcoffset;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            candidatepos = srcoffset - pos;
        }
        
        doffset = ++pos;
    }
    
    //pack results into a single int, assumes int is int32 or longer;
    int16_t* rvp = reinterpret_cast<int16_t*>(&rv);
    rvp[0] = candidatelen > CMD_OFFSET_THRESHOLD ? candidatelen : 0;
    rvp[1] = candidatepos;
    
    return rv;
}

int copyCandidate(const uint8_t* start, int srcoffset, int len)
{
    int rv;
    //current offset, this starts at srcoffset
    int coffset = srcoffset;
    //offset to start of processed or "done" bytes within range
    int doffset = 0;
    
    int16_t candidatelen = 0;
    int16_t candidatepos = -1;
    
    int pos = doffset;
    while(coffset < len && doffset < srcoffset && pos < CMD_COPY_L_MAX){
        //look for a match
        int runlen = 0;
        while(coffset < len && doffset < srcoffset && runlen < CMD_COPY_L_MAX) {
            if(start[coffset++] == start[doffset++]) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        coffset = srcoffset;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            candidatepos = pos;
        }
        
        //increment the position we look at in already processed part
        doffset = ++pos;
    }
    
    //pack results into a single int, assumes int is int32 or longer;
    int16_t* rvp = reinterpret_cast<int16_t*>(&rv);
    rvp[0] = candidatelen > CMD_OFFSET_THRESHOLD ? candidatelen : 0;
    rvp[1] = candidatepos;
    
    return rv;
}

int fillCandidate(const uint8_t* start, int srcoffset, int len)
{
    //current offset, this starts at srcoffset
    int coffset = srcoffset;
    
    int candidatelen = 1;
    uint8_t fillbyte = start[coffset++];
    
    while(coffset < len && candidatelen < CMD_FILL_MAX) {
        if(fillbyte != start[coffset++]) {
            break;
        }
        candidatelen++;
    }
    
    return candidatelen > CMD_FILL_THRESHOLD ? candidatelen : 0;
}

int xferCandidate(const uint8_t* start, int srcoffset, int len)
{
    //current offset, this starts at srcoffset
    int coffset = srcoffset;
    
    //find a long stretch of dissimilar bytes
    int candidatelen = 1;
    int runlen = 1;
    uint8_t lastbyte = start[coffset++];
    
    while(coffset < len && candidatelen < CMD_TRANSFER_MAX) {
        uint8_t nextbyte = start[coffset++];
        if(nextbyte == lastbyte) {
            runlen++;
            if(runlen > CMD_FILL_THRESHOLD) {
                candidatelen -= runlen - 2;
                break;
            }
        } else {
            runlen = 1;
        }
        candidatelen++;
        lastbyte = nextbyte;
    }
    
    return candidatelen;
}

int decode80(uint8_t* src, uint8_t* dest)
{
    uint8_t *writep = dest;
    uint8_t *readp = src;
    uint8_t *copyp;

    uint16_t count;
    /*
       1 0cccpppp p
       2 10cccccc
       3 11cccccc p p
       4 11111110 c c v
       5 11111111 c c p p
       */

    while (true) {
	uint8_t command = *readp++;
	if ((command & 0x80) == 0x00) {
	    //bit 7 = 0
            //command 0 (0cccpppp p): copy
            count = (command >> 4) + 3;
            copyp = writep - (((command & 0xf) << 8) + *readp++);
            while (count--)
                *writep++ = *copyp++;
	} else {
	    // 10cccccc (2) 
	    count = command & 0x3f;
	    if((command & 0x40) == 0) {
		// Finished decoding
		if (!count)
		    break;
		while (count--)
                        *writep++ = *readp++;
	    } else {
		if(count < 0x3e) {
		    #if __BYTE_ORDER == __BIG_ENDIAN
                    uint16_t bigend;
                    memcpy(&bigend, readp, 2);
                    copyp = &dest[le16toh(bigend)];
                    #else
                    copyp = &dest[*(unsigned short*)readp];
                    #endif

                    readp += 2;
                    while (count--)
                        *writep++ = *copyp++;
		} else if (count == 0x3e) {
		    //command 3 (11111110 c c v): fill
                    #if __BYTE_ORDER == __BIG_ENDIAN
                    memset(&count, 0, sizeof(unsigned int));
                    memcpy(&count, readp, 2);
                    count = le16toh(count);
                    #else
                    count = *(unsigned short*)readp;
                    #endif

                    readp += 2;
                    command = *readp++;
                    while (count--)
                        *writep++ = command;
		} else {
		    //command 4 (copy 11111111 c c p p): copy
                    #if __BYTE_ORDER == __BIG_ENDIAN
                    memset(&count, 0, sizeof(unsigned int));
                    memcpy(&count, readp, 2);
                    count = le16toh(count);
                    #else
                    count = *(unsigned short*)readp;
                    #endif

                    readp += 2;

                    #if __BYTE_ORDER == __BIG_ENDIAN
                    uint16_t bigend;
                    memcpy(&bigend, readp, 2);
                    copyp = &dest[le16toh(bigend)];
                    #else
                    copyp = &dest[*(unsigned short*)readp];
                    #endif

                    readp += 2;
                    while (count--)
                        *writep++ = *copyp++;
		}
	    }
	}
    }
    return (writep - dest);
}

int decode80(std::istream& src, uint8_t* dest)
{
    IStream& _stream= reinterpret_cast<IStream&>(src);
    uint8_t *writep = dest;

    uint16_t count,
	     pos;
    /*
       1 0cccpppp p
       2 10cccccc
       3 11cccccc p p
       4 11111110 c c v
       5 11111111 c c p p
       */

    while (true) {
	uint8_t command = _stream.get();
	if ((command & 0x80) == 0x00) {
	    // 0cccpppp p (1) 
	    count = ((command & 0x70) >> 4) + 3;
	    pos = (command  & 0xf) << 8 | _stream.get();
	    //FIXME: This happens at least with WSA animations from Dune 2 demo...
	    if(writep - pos < dest)
		throw(Exception(LOG_ERROR, "Decode", "Decode80 position is outside memory area (format probably not supported yet)"));
	    //printf("Cmd 1(0x%x), count: %d, pos: %d\n", command, count, pos);
	    my_memcpy(writep, writep - pos, count);
	    writep += count;
	} else {
	    // 10cccccc (2) 
	    count = command & 0x3f;
	    if((command & 0x40) == 0) {
		//printf("Cmd 2(0x%x), count: %d\n", command, count);
		// Finished decoding
		if (!count)
		    break;
		_stream.read(reinterpret_cast<char*>(writep), count);
		writep += count;
	    } else {
		if(count < 0x3e) {
		    // 11cccccc p p (3)
		    count += 3;
		    pos = _stream.getU16LE();
		    //printf("Cmd 3(0x%x), count: %d, pos: %d\n", command, count, pos);
		    my_memcpy(writep, dest + pos, count);
		    writep += count;
		    pos += count;
		} else if (count == 0x3e) {
		    // 11111110 c c v(4) 
		    count = _stream.getU16LE();
		    uint8_t color = src.get();
		    //printf("Cmd 4(0x%x), count: %d, color: %d\n", command, count, color);
		    memset(writep, color, count);
		    writep += count;
		} else {
		    // 11111111 c c p p (5)
		    count = _stream.getU16LE();
		    pos = _stream.getU16LE();
		    //printf("Cmd 5(0x%x), count: %d, pos: %d\n", command, count, pos);
		    my_memcpy(writep, dest + pos, count);
		    writep += count;
		}
	    }
	}
    }
    return (writep - dest);
}

int encode80(uint8_t* src, uint8_t* dest, int len)
{
    int srcoffset = 0;
    int destoffset = 0;
    
    //start with transfer command
    dest[destoffset++] = CMD_TRANSFER | 1;
    
    while(srcoffset < len){
        //Select best method for coming bytes
        int resultoff = offsetCopyCandidate(src, srcoffset, len);
        int resultcopy = copyCandidate(src, srcoffset, len);
        int resultfill = fillCandidate(src, srcoffset, len);
        int resultxfer = xferCandidate(src, srcoffset, len);
        //pointers to make accessing packed results easier
        uint16_t* offp = reinterpret_cast<uint16_t*>(&resultoff);
        uint16_t* cpyp = reinterpret_cast<uint16_t*>(&resultcopy);
        
        int resultbest = std::max(offp[0], cpyp[0], resultfill, resultxfer);
        
        //method 4, fill
        if(resultbest == resultfill) {
            uint8_t colourval = src[srcoffset++];
            int lefill = htole32(resultfill);
            
            dest[destoffset++] = CMD_FILL;
            memcpy(dest + destoffset, reinterpret_cast<uint8_t*>(&lefill), 2);
            destoffset += 2;
            dest[destoffset++] = colourval;
            
            srcoffset += resultfill - 1;
            
        //method 1, offset copy    
        } else if(resultbest == *reinterpret_cast<int16_t*>(&resultoff)) {
            dest[destoffset++] = (CMD_OFFSET | ((offp[0] - 3) << 4) | (offp[1] >> 8));
            uint16_t leoffp1 = htole16(offp[1]);
            dest[destoffset++] = *reinterpret_cast<uint8_t*>(&leoffp1);
               
        }
    }
    
}

int encode80(uint8_t* src, std::ostream& dest, int len)
{
    
}

} } //eastwood codec