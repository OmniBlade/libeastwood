#include "eastwood/codec/base64.h"

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

int decodeBase64(std::string src, uint8_t* dest)
{
    uint32_t i;
    uint8_t a, b, c, d;
    int bits_to_skip = 0;
    uint32_t varLength = len;
    const int8_t* srcp = src.c_str();
    uint8_t* start = dest;

    for( i = varLength-1; srcp[i] == '='; i-- ) {
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
}

} } //eastwood codec
