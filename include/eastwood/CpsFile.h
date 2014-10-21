#ifndef EASTWOOD_CPSFILE_H
#define EASTWOOD_CPSFILE_H

#include "eastwood/BaseImage.h"
#include "eastwood/Palette.h"
#include <istream>
#include <ostream>

namespace eastwood {

enum compressionFormat {
    UNCOMPRESSED = 0x000,
    FORMAT_LBM = 0x003,
    FORMAT_80 = 0x004
};

class CpsFile : public BaseImage
{
    public:
	CpsFile(std::istream &stream, Palette palette = Palette(0));

        Palette getPalette() { return _palette; }
        void setPalette(Palette pal) { _palette = pal; }
        void writeCps(std::ostream& stream);
        Surface getSurface();
    private:
	compressionFormat _format;
        Palette _palette;

};

}
#endif // EASTWOOD_CPSFILE_H
