#include "eastwood/codec/rle.h"

namespace eastwood { namespace codec {

const uint8_t RLEMARK = 0xC0;

int decodeRLE(std::istream& src, uint8_t* dest, int len)
{
    IStream& _stream= reinterpret_cast<IStream&>(src);
    int startpos = _stream.tellg();
    int endpos = startpos + len;
    int doffset = 0;
    
    while(_stream.tellg() < endpos) {
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

int encodeRLE(const uint8_t *src, std::ostream& dest, int len)
{
    OStream& _stream= reinterpret_cast<OStream&>(src);
    int startpos = _stream.tellp();
    uint8_t pixel = src[0];
    uint8_t count = 1;
    uint8_t next = src[0];
    
    for(int i = 1; i < len; i++){
        next = src[i];
        if(pixel == next) {
            count++;
            pixel = next;
        }
        
        if(count == 63 || pixel != next) {
            if(pixel < 0xC0 && count == 1){
                dest.put(pixel);
            } else {
                dest.put(count & 0xC0);
                while(count){
                    dest.put(pixel);
                    count--;
                }
            }
            pixel = next;
            count = 1;
        }
    }
    
    int endpos = _stream.tellp();
    
    return endpos - startpos;
}

} } //eastwood codec
