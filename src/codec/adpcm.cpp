#include "eastwood/codec/adpcm.h"
#include "eastwood/Log.h"
#include "eastwood/IStream.h"

#include <vector>
#include <limits>

namespace eastwood { namespace codec {

//anon namespace for helper functions
namespace {

const int steps[89] = {
        7,     8,     9,     10,    11,    12,     13,    14,    16,
        17,    19,    21,    23,    25,    28,     31,    34,    37,
        41,    45,    50,    55,    60,    66,     73,    80,    88,
        97,    107,   118,   130,   143,   157,    173,   190,   209,
        230,   253,   279,   307,   337,   371,    408,   449,   494,
        544,   598,   658,   724,   796,   876,    963,   1060,  1166,
        1282,  1411,  1552,  1707,  1878,  2066,   2272,  2499,  2749,
        3024,  3327,  3660,  4026,  4428,  4871,   5358,  5894,  6484,
        7132,  7845,  8630,  9493,  10442, 11487,  12635, 13899, 15289,
        16818, 18500, 20350, 22385, 24623, 27086,  29794, 32767
    };

const int adjusts[8] = {-1,-1,-1,-1,2,4,6,8};

template<class OutputType, class InputType>
OutputType Clip(InputType value, InputType min, InputType max)
{
    if (value < min) {
        value = min;
    } else if (value > max) {
        value = max;
    }
    return static_cast<OutputType>(value);
}

template<class OutputType, class InputType>
OutputType Clip(InputType value)
{
    return Clip<OutputType, InputType>(value, std::numeric_limits<OutputType>::min(), std::numeric_limits<OutputType>::max());
}

} //anon namespace

void decodeIMA(std::istream& src, uint8_t* dest, uint16_t compressed_size, 
               int& sample, int& index) 
{
    IStream& _stream= reinterpret_cast<IStream&>(src);
    if (compressed_size==0)
            return;

    uint16_t uncompressed_size = compressed_size * 2;

    int16_t* dest16bit = reinterpret_cast<int16_t*>(dest);

    // Each byte is processed twice: lower then upper half
    for (int samples = 0; samples < uncompressed_size; ++samples) {
        unsigned char code;
        if (samples & 1) {
            code = (_stream.get()) >> 4;
        } else {
            code = (_stream.peek()) & 0xF;
        }

        int step = steps[index];
        int delta = step >> 3;

        if (code & 1) {
            delta += step >> 2;
        }
        if (code & 2) {
            delta += step >> 1;
        }
        if (code & 4) {
            delta += step;
        }
        if (code & 8) {
            sample -= delta;
        } else {
            sample += delta;
        }

        sample = Clip<int16_t>(sample);
        *dest16bit++ = sample;

        index += adjusts[code & 7];
        index = Clip<uint8_t>(index, 0, 88);
    }
}

void decodeIMA(uint8_t* src, uint8_t* dest, uint16_t compressed_size, 
               int& sample, int& index) 
{
    if (compressed_size==0)
            return;

    uint16_t uncompressed_size = compressed_size * 2;

    int16_t* dest16bit = reinterpret_cast<int16_t*>(dest);

    // Each byte is processed twice: lower then upper half
    for (int samples = 0; samples < uncompressed_size; ++samples) {
        unsigned char code;
        if (samples & 1) {
            code = (*src++) >> 4;
        } else {
            code = (*src) & 0xF;
        }

        int step = steps[index];
        int delta = step >> 3;

        if (code & 1) {
            delta += step >> 2;
        }
        if (code & 2) {
            delta += step >> 1;
        }
        if (code & 4) {
            delta += step;
        }
        if (code & 8) {
            sample -= delta;
        } else {
            sample += delta;
        }

        sample = Clip<int16_t>(sample);
        *dest16bit++ = sample;

        index += adjusts[code & 7];
        index = Clip<uint8_t>(index, 0, 88);
    }
}

} } //eastwood codec
