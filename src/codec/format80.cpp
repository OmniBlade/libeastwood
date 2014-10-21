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
const int     CMD_OFFSET_THRESHOLD = 2;	// Must encode at least 3 uint8_ts
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

static inline void writeLE16(int16_t val, uint8_t*& writep)
{
    val = htole16(val);
    *writep++ = val & 0xff;
    *writep++ = val >> 8;
}

int offsetCopyCandidate(const uint8_t* src, const uint8_t* readp, 
                        const uint8_t* srcendp, const uint8_t*& pos)
{
    //pointer for evaluating previous uint8_ts
    const uint8_t* checkpos = readp - src > CMD_OFFSET_RANGE ? readp - CMD_OFFSET_RANGE : src;
    //pointer to mark where we started our reads from
    const uint8_t* readstartp = readp;
    
    //length of best evaluated run
    int candidatelen = 0;
    
    //mark where we started checking previous uint8_ts from
    const uint8_t* initpos = checkpos;
    while(readp < srcendp && checkpos < readstartp) {
        //look for a match
        int runlen = 0;
        while(readp < srcendp && checkpos < readstartp && runlen < CMD_OFFSET_MAX) {
            if(*readp++ == *checkpos++) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        readp = readstartp;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            //set the pointer we passed in to where we started this promising run
            pos = initpos;
        }
        
        //reset our checking pointer to the next uint8_t along
        checkpos = ++initpos;
    }
    
    return candidatelen > CMD_OFFSET_THRESHOLD ? candidatelen : 0;
}

int copyCandidate(const uint8_t* src, const uint8_t* readp, 
                   const uint8_t* srcendp, const uint8_t*& pos)
{
    //pointer for evaluating previous uint8_ts
    const uint8_t* checkpos = src;
    //pointer to mark where we started our reads from
    const uint8_t* readstartp = readp;
    const uint8_t* checkendp = readstartp - src < CMD_COPY_L_MAX * 2 ? readstartp : src + CMD_COPY_L_MAX * 2;
    
    //length of best evaluated run
    int candidatelen = 0;
    
    const uint8_t* initpos = checkpos;
    
    while(readp < srcendp && checkpos < checkendp && checkpos - src < CMD_COPY_L_MAX){
        //look for a match
        int runlen = 0;
        while(readp < srcendp && checkpos < checkendp && runlen < CMD_COPY_L_MAX) {
            if(*readp++ == *checkpos++) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        readp = readstartp;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            pos = initpos;
        }
        
        //increment the position we look at in already processed part
        checkpos = ++initpos;
    }
    
    return candidatelen > CMD_COPY_S_THRESHOLD ? candidatelen : 0;
}

int fillCandidate(const uint8_t* current, const uint8_t* end)
{
    //current offset, this starts at srcoffset
    
    int candidatelen = 1;
    uint8_t filluint8_t = *current++;
    
    while(current < end && candidatelen < CMD_FILL_MAX) {
        if(filluint8_t != *current++) {
            break;
        }
        candidatelen++;
    }
    
    return candidatelen > CMD_FILL_THRESHOLD ? candidatelen : 0;
}

int xferCandidate(const uint8_t* current, const uint8_t* end)
{   
    //find a long stretch of dissimilar uint8_ts
    int candidatelen = 1;
    int runlen = 1;
    uint8_t lastuint8_t = *current++;
    
    while(current < end && candidatelen < CMD_TRANSFER_MAX) {
        uint8_t nextuint8_t = *current++;
        if(nextuint8_t == lastuint8_t) {
            runlen++;
            if(runlen > CMD_FILL_THRESHOLD) {
                candidatelen -= runlen - 2;
                break;
            }
        } else {
            runlen = 1;
        }
        candidatelen++;
        lastuint8_t = nextuint8_t;
    }
    
    return candidatelen;
}

int decode80(const uint8_t* src, uint8_t* dest)
{
    uint8_t *writep = dest;
    const uint8_t *readp = src;
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
    return writep - dest;
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
    return writep - dest;
}

#if 0
int encode80(const uint8_t* src, uint8_t* dest, int len)
{
    uint8_t* writep = dest;
    const uint8_t* readp = src;
    const uint8_t* srcendp = src + len;
    //int srcoffset = readp - src;
    
    int temp[2];
    
    //start with transfer command
    //*writep++ = CMD_TRANSFER | 1;
    
    while(readp < srcendp){
        //Select best method for coming uint8_ts
        const uint8_t* offposp = NULL;
        int resultoff = offsetCopyCandidate(src, readp, srcendp, offposp);
        //again copy results to temp
        const uint8_t* cpyposp = NULL;
        int resultcopy = copyCandidate(src, readp, srcendp, cpyposp);
        //these only return one result so direct assign
        int resultfill = fillCandidate(readp, srcendp);
        int resultxfer = xferCandidate(readp, srcendp);
        
        LOG_DEBUG("offcopy len: %d pos: %d", resultoff, readp - offposp);
        LOG_DEBUG("copy len: %d pos: %d", resultcopy, cpyposp - src);
        LOG_DEBUG("fill len: %d", resultfill);
        LOG_DEBUG("xfer len: %d", resultxfer);
        
        int resultbest = std::max(std::max(resultoff, resultcopy), 
                                  std::max(resultfill, resultxfer));
        
        //method 4, fill
        if(resultbest == resultfill) {
            uint8_t colourval = *readp++;
            
            *writep++ = CMD_FILL;
            writeLE16(resultfill, writep);
            *writep++ = colourval;
            
            readp += resultfill;
            
        //method 1, offset copy    
        } else if(resultbest == resultoff) {
            int offpos = readp - offposp;
            *writep++ = (resultoff - 3) << 4 | offpos >> 8;
            *writep++ = offpos & 0xff;
            
            readp += resultoff;
        } else if (resultbest == resultcopy) {
            
            int copypos = cpyposp - src;
            
            // method 3 - small copy
            if (resultcopy <= CMD_COPY_S_MAX) {
                *writep++ = CMD_COPY_S | (resultcopy - 3);
                writeLE16(copypos, writep);
            // method 5 - large copy
            } else {
                *writep++ = CMD_COPY_L;
                writeLE16(resultcopy, writep);
                writeLE16(copypos, writep);
            }
            readp += resultcopy;
        // method 2 uint8_ts from source to writep
        } else {
            *writep++ = CMD_TRANSFER | resultxfer;
            memcpy(writep, readp, resultxfer);
            writep += resultxfer;
            readp += resultxfer;
        }
        
        //get our current offset in source
        //srcoffset = readp - src;
    }
    // format80 data must end with 0x80
    *writep++ = CMD_TRANSFER;
    //return length of data after compression
    return writep - dest;
}
#endif

int get_same(const uint8_t* src, const uint8_t* readp, const uint8_t* srcendp, const uint8_t*& pos)
{
    //pointer for evaluating previous uint8_ts
    const uint8_t* checkpos = src;
    //pointer to mark where we started our reads from
    const uint8_t* readstartp = readp;
    const uint8_t* checkendp = readstartp;
    
    //length of best evaluated run
    int candidatelen = 0;
    
    const uint8_t* initpos = checkpos;
    
    while(readp < srcendp && checkpos < checkendp){
        //look for a match
        int runlen = 0;
        while(readp < srcendp && checkpos < checkendp) {
            if(*readp++ == *checkpos++) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        readp = readstartp;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            pos = initpos;
        }
        
        //increment the position we look at in already processed part
        checkpos = ++initpos;
    }
    
    return candidatelen;
}

int get_run_length(const uint8_t* r, const uint8_t* s_end)
{
	int count = 1;
	int v = *r++;
	while (r < s_end && *r++ == v)
		count++;
	return count;
}

static void write80_c0(uint8_t*& w, int count, int p)
{
    *w++ = (count - 3) << 4 | p >> 8;
    *w++ = p & 0xff;
}

static void write80_c1(uint8_t*& w, int count, const uint8_t* r)
{
    do
    {
        int c_write = count < 0x40 ? count : 0x3f;
        *w++ = 0x80 | c_write;
        memcpy(w, r, c_write);
        r += c_write;
        w += c_write;
        count -= c_write;
    }
    while (count);
}

static void write80_c2(uint8_t*& w, int count, int p)
{
    *w++ = 0xc0 | (count - 3);
    writeLE16(p, w);
}

static void write80_c3(uint8_t*& w, int count, int v)
{
    *w++ = 0xfe;
    writeLE16(count, w);
    *w++ = v;
}

static void write80_c4(uint8_t*& w, int count, int p)
{
    *w++ = 0xff;
    writeLE16(count, w);
    writeLE16(p, w);
}

static void flush_c1(uint8_t*& w, const uint8_t* r, const uint8_t*& copy_from)
{
    if (copy_from)
    {
        write80_c1(w, r - copy_from, copy_from);
        copy_from = NULL;
    }
}

int encode80(const uint8_t* src, uint8_t* dest, int len)
{
    // full compression
    const uint8_t* s_end = src + len;
    const uint8_t* readp = src;
    uint8_t* writep = dest;
    const uint8_t* copy_from = NULL;
    *writep++ = -127;
    *writep++ = *readp++;    
    while (readp < s_end)
    {
        const uint8_t* pos;
        int blocksize = get_same(src, readp, s_end, pos);
        int runlen = get_run_length(readp, s_end);
        if (runlen < blocksize && blocksize > 2)
        {
            flush_c1(writep, readp, copy_from);
            if (blocksize - 3 < 8 && readp - pos < 0x1000)
                write80_c0(writep, blocksize, readp - pos);
            else if (blocksize - 3 < 0x3e)
                write80_c2(writep, blocksize, pos - src);
            else 
                write80_c4(writep, blocksize, pos - src);				
            readp += blocksize;
        }
        else
        {
            if (runlen < 3)
            {
                if (!copy_from)
                    copy_from = readp;
            }
            else
            {
                flush_c1(writep, readp, copy_from);
                write80_c3(writep, runlen, *readp);
            }
            readp += runlen;
        }
    }
    flush_c1(writep, readp, copy_from);
    write80_c1(writep, 0, NULL);
    return writep - dest;
}

int encode80(const uint8_t* src, std::ostream& dest, int len)
{
    OStream& _stream= reinterpret_cast<OStream&>(dest);
    std::vector<uint8_t> buf(len);
    int compressed = encode80(src, &buf.at(0), len);
    _stream.write(reinterpret_cast<char*>(&buf.at(0)), compressed);
    return compressed;
}

} } //eastwood codec