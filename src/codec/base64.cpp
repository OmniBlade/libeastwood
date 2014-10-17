#include "eastwood/codec/base64.h"
#include "eastwood/Log.h"

namespace eastwood { namespace codec {

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

const static char etable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                              'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                              'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                              'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                              'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                              'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                              'w', 'x', 'y', 'z', '0', '1', '2', '3',
                              '4', '5', '6', '7', '8', '9', '+', '/'};

const static char PAD = '=';

int decodeBase64(std::string src, uint8_t* dest)
{
    uint32_t i;
    uint8_t a, b, c, d;
    int bits_to_skip = 0;
    uint32_t varLength = src.size();
    const char* srcp = src.c_str();
    uint8_t* start = dest;

    for(i = varLength-1; srcp[i] == PAD; i--) {
        bits_to_skip += 2;
        varLength--;
    }
    
    if( bits_to_skip >= 6 ) {
        LOG_WARNING("Error in base64 (too many '=').");
        return -1;
    }

    while (varLength >= 4) {
        a = dtable[srcp[0]];
        b = dtable[srcp[1]];
        c = dtable[srcp[2]];
        d = dtable[srcp[3]];
        if( a == 0x80 || b == 0x80 ||
                c == 0x80 || d == 0x80 ) {
            LOG_WARNING("Illegal character.");
        }
        dest[0] = a << 2 | b >> 4;
        dest[1] = b << 4 | c >> 2;
        dest[2] = c << 6 | d;
        dest+=3;
        varLength-=4;
        srcp += 4;
    }

    if( varLength > 0 )
    {
        if( bits_to_skip == 4 && varLength == 2 ) {
            a = dtable[srcp[0]];
            b = dtable[srcp[1]];

            dest[0] = a << 2 | b >> 4;
        } else if( bits_to_skip == 2 && varLength == 3 ) {
            a = dtable[srcp[0]];
            b = dtable[srcp[1]];
            c = dtable[srcp[2]];

            dest[0] = a << 2 | b >> 4;
            dest[1] = b << 4 | c >> 2;
        } else {
            LOG_WARNING("Error in base64. #bits to skip doesn't match length.");
            LOG_WARNING("skip %d bits, %d chars left \"%s\"", bits_to_skip, varLength, src);
            return -1;
        }
    }

    return dest - start;
}

std::string encodeBase64(uint8_t* src, int len)
{
    std::string encstring;
    encstring.reserve(((len/3) + (len % 3 > 0)) * 4);
    int temp;
    uint8_t* cursor;
    
    for(int i = 0; i < len / 3; i++){
        temp  = (*cursor++) << 16;
        temp += (*cursor++) << 8;
        temp += (*cursor++);
        encstring.append(1,etable[(temp & 0x00FC0000) >> 18]);
        encstring.append(1,etable[(temp & 0x0003F000) >> 12]);
        encstring.append(1,etable[(temp & 0x00000FC0) >> 6 ]);
        encstring.append(1,etable[(temp & 0x0000003F)      ]);    
    }
    
    switch(len % 3) {
    case 1:
        temp = (*cursor++) << 16;
        encstring.append(1,etable[(temp & 0x00FC0000) >> 18]);
        encstring.append(1,etable[(temp & 0x0003F000) >> 12]);
        encstring.append(2,PAD);
        break;
    case 2:
        temp  = (*cursor++) << 16;
        temp += (*cursor++) << 8;
        encstring.append(1,etable[(temp & 0x00FC0000) >> 18]);
        encstring.append(1,etable[(temp & 0x0003F000) >> 12]);
        encstring.append(1,etable[(temp & 0x00000FC0) >> 6 ]);
        encstring.append(1,PAD);
        break;
    default:
        break;
    }
    
    return encstring;
}

} } //eastwood codec
