#include "eastwood/VqpFile.h"
#include "eastwood/IStream.h"
#include "eastwood/adl/fmopl.h"

namespace eastwood {

VqpFile::VqpFile(std::istream& stream)
    : _lookuptable()
{
    IStream& _stream = reinterpret_cast<IStream&>(stream);
    
    //WSA "pal" file lookuptables
    if(_stream.sizeg() == 65536) {
        _lookuptable.push_back(std::vector<uint8_t>(65536));
        _stream.read(reinterpret_cast<char*>(&_lookuptable.back().at(0)), 65536);
    //Proper vqp files    
    } else {
        int count = _stream.getU32LE();
        for(int i = 0; i < count; i++) {
            _lookuptable.push_back(std::vector<uint8_t>(65536));
            for(int c1 = 0; c1 < 256; c1++) {
                for(int c2 = 0; c2 <= c1; c2++) {
                    uint8_t val = _stream.get();
                    _lookuptable[i][(c2 << 8) + c1] =  _lookuptable[i][(c1 << 8) + c2] = val;
                }
            }
        }
    }
}

VqpFile::VqpFile(Palette pal)
    : _lookuptable()
{
    _lookuptable.push_back(std::vector<uint8_t>(65536));
    createTable(pal);
}

uint8_t VqpFile::getAverage(uint8_t a, uint8_t b, int table)
{
    return _lookuptable[table][(*reinterpret_cast<int*>(&a) << 8)  + *reinterpret_cast<int*>(&b)];
}

inline Color VqpFile::getAverage(Color a, Color b)
{
    uint32_t cola = le32toh(*reinterpret_cast<uint32_t*>(&a));
    uint32_t colb = le32toh(*reinterpret_cast<uint32_t*>(&b));
    uint32_t rv = (((cola ^ colb) & 0xfffefefeUL) >> 1) + (cola & colb);
    return *reinterpret_cast<Color*>(htole32(rv));
}

void VqpFile::createTable(Palette pal)
{
    int x, y;
    Color m;
    
    for(y = 0; y < 256; y++) {
        for(x = 0; x < 256; x++) {
            m.r = ((int)pal[x].r + (int)pal[y].r) / 2;
            m.g = ((int)pal[x].g + (int)pal[y].g) / 2;
            m.b = ((int)pal[x].b + (int)pal[y].b) / 2;
            _lookuptable[0][(y << 8) + x] =  _lookuptable[0][(x << 8) + y] = pal.nearest(m);
        }
    }
}

} //eastwood
