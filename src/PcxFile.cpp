#include "eastwood/PcxFile.h"
#include "eastwood/StdDef.h"
#include "eastwood/Exception.h"
#include "eastwood/PalFile.h"


namespace eastwood {

const uint8_t pcxSig = 0x0A;
const int OFFSET = 128;

PcxFile::PcxFile(CCFileClass& fclass):
Surface(), _format(V30_STD)
{
    uint16_t x_min, 
             y_min, 
             x_max, 
             y_max;
    uint16_t linebytes;
    
    if(fclass.read8() != pcxSig)
        throw(Exception(LOG_ERROR, "PcxFile", "Not a zSoft PCX"));
    
    _format = static_cast<formatVersion>(fclass.read8());
    if(_format != V30_STD)
        throw(Exception(LOG_ERROR, "PcxFile", "Formats other than 5 not handled"));
    
    if(fclass.read8() != 1)
        throw(Exception(LOG_ERROR, "PcxFile", "Encoding byte not correct"));
    
    _bpp = fclass.read8();
    if(_bpp < 8)
        throw(Exception(LOG_ERROR, "PcxFile", "PCX of bit depth < 8 not handled"));
    _Bpp = _bpp / 8;
    
    x_min = fclass.readle16();
    y_min = fclass.readle16();
    x_max = fclass.readle16();
    y_max = fclass.readle16();
    
    //ignore resolution and 16 color palette
    fclass.seek(53, SEEK_CUR);
    
    //again make sure we have a 256 colour image
    if(fclass.read8() != 1)
        throw(Exception(LOG_ERROR, "PcxFile", "Bitplanes > 1 not supported"));
    
    linebytes = fclass.readle16();
    _height = (y_max - y_min) + 1;
    _width = _pitch = linebytes;
    
    LOG_DEBUG("Height is %d and Width is %d for total size %d", _height, _width, size());
    
    fclass.seek(fclass.getSize() - 768, SEEK_SET);
    PalFile pal(fclass, false);
    _palette = pal.getPalette();
    LOG_DEBUG("Palette size %d", _palette.size());
    
    _pixels = new uint8_t[_height * _pitch];
    fclass.seek(OFFSET, SEEK_SET);
    getPixels(fclass);
}

void PcxFile::getPixels(CCFileClass& fclass)
{
    unsigned char Pixel;
    unsigned char Number;
    unsigned int y, x;
    long Index;
    unsigned int offset;


    uint32_t Height = _height;
    uint32_t Width = _width;

    /* RLE (Run Length Encoding) */
    offset = 128;
    Index = 0;
    y = 0;
    while( y < Height ) {
        x = 0;
        while( x < Width ){
            Pixel = fclass.read8();
            if( Pixel > 192 ){
                Number = Pixel-192;
                Pixel = fclass.read8();
                for( unsigned int i=0; i<Number; i++ ){
                        _pixels[Index++] = Pixel;
                        x++;
                }
            }else{
                _pixels[Index++] = Pixel;
                x++;
            }
        }
        y++;
    }
}

PcxFile::~PcxFile() {
    
}

} //eastwood
