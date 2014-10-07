#include "eastwood/Decode.h"
#include "eastwood/StdDef.h"
#include "eastwood/Log.h"
#include "eastwood/Exception.h"
#include <memory>

namespace eastwood { namespace Decode {

//base 64 decode table
const static char dtable[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

inline void my_memcpy(uint8_t *dst, const uint8_t *src, uint16_t cnt)
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

int decodeBase64(const uint8_t* src, uint8_t* dest, const uint32_t length)
{
    uint32_t i;
    uint8_t a, b, c, d;
    int bits_to_skip = 0;
    uint32_t varLength = length;

    for( i = varLength-1; src[i] == '='; i-- ) {
        bits_to_skip += 2;
        varLength--;
    }
    
    if( bits_to_skip >= 6 ) {
        LOG_WARNING("Error in base64 (too many '=').");
        return -1;
    }

    while (varLength >= 4) {
        a = dtable[src[0]];
        b = dtable[src[1]];
        c = dtable[src[2]];
        d = dtable[src[3]];
        if( a == 0x80 || b == 0x80 ||
                c == 0x80 || d == 0x80 ) {
            LOG_WARNING("Illegal character.");
        }
        dest[0] = a << 2 | b >> 4;
        dest[1] = b << 4 | c >> 2;
        dest[2] = c << 6 | d;
        dest+=3;
        varLength-=4;
        src += 4;
    }

    if( varLength > 0 )
    {
        if( bits_to_skip == 4 && varLength == 2 ) {
            a = dtable[src[0]];
            b = dtable[src[1]];

            dest[0] = a << 2 | b >> 4;
        } else if( bits_to_skip == 2 && varLength == 3 ) {
            a = dtable[src[0]];
            b = dtable[src[1]];
            c = dtable[src[2]];

            dest[0] = a << 2 | b >> 4;
            dest[1] = b << 4 | c >> 2;
        } else {
            LOG_WARNING("Error in base64. #bits to skip doesn't match length.");
            LOG_WARNING("skip %d bits, %d chars left \"%s\"", bits_to_skip, varLength, src);
            return -1;
        }
    }

    return 0;
}

int decode80(CCFileClass& fclass, uint8_t *image_out, uint32_t checksum)
{
    uint8_t *writep = image_out;

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
	uint8_t command = fclass.read8();
	if ((command & 0x80) == 0x00) {
	    // 0cccpppp p (1) 
	    count = ((command & 0x70) >> 4) + 3;
	    pos = (command  & 0xf) << 8 | fclass.read8();
	    //FIXME: This happens at least with WSA animations from Dune 2 demo...
	    if(writep - pos < image_out)
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
		fclass.read(reinterpret_cast<char*>(writep), count);
		writep += count;
	    } else {
		if(count < 0x3e) {
		    // 11cccccc p p (3)
		    count += 3;
		    pos = fclass.readle16();
		    //printf("Cmd 3(0x%x), count: %d, pos: %d\n", command, count, pos);
		    my_memcpy(writep, image_out + pos, count);
		    writep += count;
		    pos += count;
		} else if (count == 0x3e) {
		    // 11111110 c c v(4) 
		    count = fclass.readle16();
		    uint8_t color = fclass.read8();
		    //printf("Cmd 4(0x%x), count: %d, color: %d\n", command, count, color);
		    memset(writep, color, count);
		    writep += count;
		} else {
		    // 11111111 c c p p (5)
		    count = fclass.readle16();
		    pos = fclass.readle16();
		    //printf("Cmd 5(0x%x), count: %d, pos: %d\n", command, count, pos);
		    my_memcpy(writep, image_out + pos, count);
		    writep += count;
		}
	    }
	}
    }
    if ((writep - image_out) != checksum)
	return -1;
    return 0;
}

int decode80buffer(const uint8_t* src, uint8_t* dest)
{
	// To resume :
	//    0 copy 0cccpppp p
	//    1 copy 10cccccc
	//    2 copy 11cccccc p p
	//    3 fill 11111110 c c v
	//    4 copy 11111111 c c p p


    const uint8_t* copyp;
    const uint8_t* readp = src;
    uint8_t* writep = dest;
    uint32_t code;
    uint32_t count;
#if __BYTE_ORDER == __BIG_ENDIAN

    uint16_t bigend; // temporary big endian var
#endif

    while (1) {
        code = *readp++;
        if (~code & 0x80) {
            //bit 7 = 0
            //command 0 (0cccpppp p): copy
            count = (code >> 4) + 3;
            copyp = writep - (((code & 0xf) << 8) + *readp++);
            while (count--)
                *writep++ = *copyp++;
        } else {
            //bit 7 = 1
            count = code & 0x3f;
            if (~code & 0x40) {
                //bit 6 = 0
                if (!count)
                    //end of image
                    break;
                //command 1 (10cccccc): copy
                while (count--)
                    *writep++ = *readp++;
            } else {
                //bit 6 = 1
                if (count < 0x3e) {
                    //command 2 (11cccccc p p): copy
                    count += 3;
                    
#if __BYTE_ORDER == __BIG_ENDIAN

                    memcpy(&bigend, readp, 2);
                    copyp = &dest[endian_bswap16(bigend)];
#else

                    copyp = &dest[*(uint16_t*)readp];
#endif
                    readp += 2;
                    while (count--)
                        *writep++ = *copyp++;
                } else if (count == 0x3e) {
                    //command 3 (11111110 c c v): fill
#if __BYTE_ORDER == __BIG_ENDIAN
                    memset(&count, 0, sizeof(uint32_t));
                    memcpy(&count, readp, 2);
                    count = endian_bswap32(count);
#else

                    count = *(uint16_t*)readp;
#endif

                    readp += 2;
                    code = *readp++;
                    while (count--)
                        *writep++ = code;
                } else {
                    //command 4 (copy 11111111 c c p p): copy
#if __BYTE_ORDER == __BIG_ENDIAN
                    memset(&count, 0, sizeof(uint32_t));
                    memcpy(&count, readp, 2);
                    count = endian_bswap32(count);
#else

                    count = *(uint16_t*)readp;
#endif

                    readp += 2;
#if __BYTE_ORDER == __BIG_ENDIAN

                    memcpy(&bigend, readp, 2);
                    copyp = &dest[endian_bswap16(bigend)];
#else

                    copyp = &dest[*(uint16_t*)readp];
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

int decode40(const uint8_t* src, uint8_t* dest)
{
	//
	//----------
	// Format40
	//----------
	//
	//As I said before the images in Format40 must be xor-ed over a previous image,
	//or against a black screen (as in the .WSA format).
	//It is used when there are only minor changes between an image and a following
	//one.
	//
	//Here I'll assume that the old image is in Dest, and that the Dest pointer is
	//set to the beginning of that buffer.
	//
	//As for the Format80, there are many commands :
	//
	//
	//(1) 1 byte
	//               byte
	//  +---+---+---+---+---+---+---+---+
	//  | 1 |   |   |   |   |   |   |   |
	//  +---+---+---+---+---+---+---+---+
	//      \___________________________/
	//                   |
	//                 Count
	//
	//  Skip count bytes in Dest (move the pointer forward).
	//
	//(2) 3 bytes
	//              byte                           word
	//  +---+---+---+---+---+---+---+---+  +---+-----+-------+
	//  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |  | 0 | ... |       |
	//  +---+---+---+---+---+---+---+---+  +---+-----+-------+
	//                                         \_____________/
	//                                                |
	//                                              Count
	//
	//  Skip count bytes.
	//
	//(3) 3 bytes
	//                byte                              word
	//  +---+---+---+---+---+---+---+---+  +---+---+-----+-------+
	//  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |  | 1 | 0 | ... |       |
	//  +---+---+---+---+---+---+---+---+  +---+---+-----+-------+
	//                                             \_____________/
	//                                                   |
	//                                                 Count
	//
	// Xor next count bytes. That means xor count bytes from Source with bytes
	// in Dest.
	//
	//(4) 4 bytes
	//              byte                               word           byte
	//  +---+---+---+---+---+---+---+---+  +---+---+-----+-------+  +-------+
	//  | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |  | 1 | 1 | ... |       |  |       |
	//  +---+---+---+---+---+---+---+---+  +---+---+-----+-------+  +-------+
	//                                             \_____________/    value
	//                                                   |
	//                                                 Count
	//
	//  Xor next count bytes in Dest with value.
	//
	//5) 1 byte
	//               byte
	//  +---+---+---+---+---+---+---+---+
	//  | 0 |   |   |   |   |   |   |   |
	//  +---+---+---+---+---+---+---+---+
	//      \___________________________/
	//                   |
	//                 Count
	//
	//  Xor next count bytes from source with dest.
	//
	//6) 3 bytes
	//              byte                     byte       byte
	//  +---+---+---+---+---+---+---+---+  +-------+  +-------+
	//  | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |  |       |  |       |
	//  +---+---+---+---+---+---+---+---+  +-------+  +-------+
	//                                       Count      Value
	//
	//  Xor next count bytes with value.
	//
	//
	//All images end with a 80h 00h 00h command.
	//
	//I think these are all the commands, but there might be some other.
	//If you find anything new, please e-mail me.
	//
	//As before here's some code :
	//
	//  DP = destination pointer
	//  SP = source pointer
	//  Source is buffer containing the Format40 data
	//  Dest   is the buffer containing the image over which the second has
	//         to be xor-ed
	//
	//
	//  SP:=0;
	//  DP:=0;
	//  repeat
	//    Com:=Source[SP];
	//    Inc(SP);
	//
	//    if (Com and $80)<>0 then {if bit 7 set}
	//    begin
	//      if Com<>$80 then  {small skip command (1)}
	//      begin
	//        Count:=Com and $7F;
	//        Inc(DP,Count);
	//      end
	//      else  {Big commands}
	//      begin
	//        Count:=Word(Source[SP]);
	//        if Count=0 then break;
	//        Inc(SP,2);
	//
	//        Tc:=(Count and $C000) shr 14;  {Tc=two topmost bits of count}
	//
	//        case Tc of
	//          0,1 : begin  {Big skip (2)}
	//                  Inc(DP,Count);
	//                end;
	//          2 : begin {big xor (3)}
	//                Count:=Count and $3FFF;
	//                for i:=1 to Count do
	//                begin
	//                  Dest[DP]:=Dest[DP] xor Source[SP];
	//                  Inc(DP);
	//                  Inc(SP);
	//                end;
	//              end;
	//          3 : begin  {big repeated xor (4)}
	//                Count:=Count and $3FFF;
	//                b:=Source[SP];
	//                Inc(SP);
	//                for i:=1 to Count do
	//                begin
	//                  Dest[DP]:=Dest[DP] xor b;
	//                  Inc(DP);
	//                end;
	//              end;
	//        end;
	//      end;
	//    end else  {xor command}
	//    begin
	//      Count:=Com;
	//      if Count=0 then
	//      begin {repeated xor (6)}
	//        Count:=Source[SP];
	//        Inc(SP);
	//        b:=Source[SP];
	//        Inc(SP);
	//        for i:=1 to Count do
	//        begin
	//          Dest[DP]:=Dest[DP] xor b;
	//          Inc(DP);
	//        end;
	//      end else  {copy xor (5)}
	//        for i:=1 to Count do
	//        begin
	//          Dest[DP]:=Dest[DP] xor Source[SP];
	//          Inc(DP);
	//          Inc(SP);
	//        end;
	//    end;
	//  until false;
	//

	// To Resume :
	//    0 fill 00000000 c v
	//    1 copy 0ccccccc
	//    2 skip 10000000 c 0ccccccc
	//    3 copy 10000000 c 10cccccc
	//    4 fill 10000000 c 11cccccc v
	//    5 skip 1ccccccc


    const uint8_t* readp = src;
    uint8_t* writep = dest;
    uint32_t code;
    uint32_t count;

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
#if __BYTE_ORDER == __BIG_ENDIAN
                memset(&count, 0, sizeof(uint32_t));
                memcpy(&count, readp, 2);
                count = endian_bswap32(count);
#else

                count = *(uint16_t*)readp;
#endif

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

/**
 * Decompress format 20 compressed data.
 *
 * @param src compressed data.
 * @param dest pointer to put uncompressed data in.
 * @param size size of compressed data?
 * @return size of uncompressed data?
 */
int decode20(const uint8_t* src, uint8_t* dest, int size)
{
    const uint8_t* r = src;
    const uint8_t* r_end = src + size;
    uint8_t* w = dest;

    while (r < r_end)
    {
        int v = *r++;
        if (v)
        {
            *w++ = v;
        }
        else
        {
            v = *r++;
            memset(w, 0, v);
            w += v;
        }
    }

    return w - dest;
}

} }
