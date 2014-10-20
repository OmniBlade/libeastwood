#include "eastwood/codec/rle.h"

namespace eastwood { namespace codec {

const uint8_t RLEMARK = 0xC0;

int decodeRLE(std::istream& src, uint8_t* dest)
{
    IStream& _stream= reinterpret_cast<IStream&>(src);
    int startpos = _stream.tellg();
    int doffset = 0;
    
    while(!_stream.eof()) {
        uint8_t value = _stream.get();
        
        if((value & RLEMARK) == RLEMARK){
            uint8_t count = value & ~RLEMARK;
            uint8_t copy = _stream.get();
            
            while(count--) {
                dest[doffset++] = copy;
            }
        } else {
            dest[doffset++] = value;
        }
    }
    
    return doffset;
}

int encodeRLE(const uint8_t* src, std::ostream& dest, int _x, int y)
{
    OStream& _stream= reinterpret_cast<OStream&>(dest);
    int startpos = _stream.tellp();
    const uint8_t* readp = src;
    //uint8_t pixel = *readp++;
    
    _x--;
    while(y--) {
        int count = 1;
        int x = _x;
        uint8_t last = *readp++;
        
        while(x--) {
            uint8_t cur = *readp++;
            if(last == cur) {
                count++;
            } else {
                while(count){
                    if (count == 1 && last < 0xc0) {
                        _stream.put(last);
                        count = 0;
                    } else {
                        const int c_write = count > 63 ? 63 : count;
                        _stream.put(0xc0 | c_write);
                        _stream.put(last);
                        count -= c_write;
                    }
                }
                count = 1;
                last = cur;
            }
            
        }
        while(count){
            if (count == 1 && last < 0xC0) {
                _stream.put(last);
                count = 0;
            } else {
                const int c_write = count > 63 ? 63 : count;
                _stream.put(0xc0 | c_write);
                _stream.put(last);
                count -= c_write;
            }
        }
    }
    
    int endpos = _stream.tellp();
    
    return endpos - startpos;
}

} } //eastwood codec
