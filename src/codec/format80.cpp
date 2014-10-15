#include "eastwood/codec/format80.h"
#include "eastwood/Exception.h"

namespace eastwood { namespace codec {

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

int encode80(uint8_t* src, uint8_t* dest)
{
    
}

int encode80(uint8_t* src, std::ostream& dest)
{
    
}

} } //eastwood codec