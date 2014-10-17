#include "eastwood/codec/format2.h"

#include <algorithm>

namespace eastwood { namespace codec {

const uint8_t CMD_FILL = 0;
const uint8_t CMD_FILL_VAL = 0;

int decode2(std::istream& src, uint8_t *dest, int len)
{
    IStream& _stream= reinterpret_cast<IStream&>(src);
    int startpos = _stream.tellg();
    int curpos = _stream.tellg();
    int doffset = 0;
    
    while(curpos - startpos < len) {
        uint8_t command = _stream.get();
        
        //fill 0s
        if(command == CMD_FILL) {
            int count = _stream.get() & 0xFF;
            while(count--) {
                dest[doffset++] = CMD_FILL_VAL;
            }
        } else {
            dest[doffset++] = command;
        }
        curpos = _stream.tellg();
    }
    
    return doffset;
}

int encode2(uint8_t* src, std::ostream& dest, int len)
{
    OStream& _stream = reinterpret_cast<OStream&>(dest);
    
    int startpos = _stream.tellp();
    int count = 0;
    int limit = std::min(len, 255);
    int soffset = 0;
    
    while(soffset < len){
        uint8_t val = src[soffset++];
        
        //count a series of 0s
        while(val == CMD_FILL_VAL) {
            while(val == CMD_FILL_VAL && count < limit) {
                count++;
                if(soffset < len) {
                    val = src[soffset++];
                } else {
                    break;
                }
            }
            _stream.put(CMD_FILL);
            _stream.put(count);
            if(soffset >= len)  break;
        }
        if(soffset >= len)  break;
        _stream.put(val);
    }
    
    int endpos = _stream.tellp();
    
    return endpos - startpos;
}

} } //eastwood codec
