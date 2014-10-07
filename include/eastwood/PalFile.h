#ifndef EASTWOOD_PALFILE_H
#define EASTWOOD_PALFILE_H

#include "eastwood/Palette.h"

namespace eastwood {

class PalFile
{
    public:
	PalFile(CCFileClass& fclass);
	~PalFile();

	Palette getPalette();

    private:
	Palette _palette;

};

}
#endif // EASTWOOD_PALFILE_H
