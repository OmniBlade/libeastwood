#ifndef EASTWOOD_PCXFILE_H
#define	EASTWOOD_PCXFILE_H

#include "Palette.h"
#include "BaseImage.h"
#include "IStream.h"

namespace eastwood {

enum formatVersion {
    PBRUSH_DOS= 0x00,
    V28_PAL = 0x02,
    V28_NOPAL = 0x03,
    PBRUSH_WIN = 0x04,
    V30_STD = 0x05
};

class PcxFile : public BaseImage
{
private:
    struct PcxHeader {
            uint8_t sig;
            uint8_t ver;
            uint8_t enc;
            uint8_t bpp;
            uint16_t xmin;
            uint16_t ymin;
            uint16_t xmax;
            uint16_t ymax;
            uint16_t hres;
            uint16_t vres;
            uint8_t pal16[48];
            uint8_t res;
            uint8_t planes;
            uint16_t bpl;
            uint16_t paltype;
            uint8_t dummy[58];
        };
        
public:
    PcxFile(std::istream &stream);
    ~PcxFile();

    Palette getPalette() { return _palette; }
    void setPalette(Palette pal) { _palette = pal; }
    void writePcx(std::ostream& stream);

private:
    void writeHeader(std::ostream& stream);
    PcxHeader _header;
};

}
#endif	/* EASTWOOD_PCXFILE_H */

