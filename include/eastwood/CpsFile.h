#ifndef EASTWOOD_CPSFILE_H
#define EASTWOOD_CPSFILE_H

#include "eastwood/Palette.h"
#include "eastwood/Surface.h"

namespace eastwood {

enum compressionFormat {
    UNCOMPRESSED = 0x000,
    FORMAT_LBM = 0x003,
    FORMAT_80 = 0x004
};

class CpsFile : public Surface
{
    public:
	CpsFile(CCFileClass& fclass, Palette palette = Palette(0));
	~CpsFile();

    private:
	void readHeader();
	compressionFormat _format;
};

}
#endif // EASTWOOD_CPSFILE_H
