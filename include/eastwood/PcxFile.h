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
    public:
	PcxFile(std::istream &stream);
	~PcxFile();
        
        Palette getPalette() { return _palette; }
        void setPalette(Palette pal) { _palette = pal; }
        void writePcx(std::ostream& stream);

    private:
	formatVersion _format;
};

}
#endif	/* EASTWOOD_PCXFILE_H */

