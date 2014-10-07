/* 
 * File:   PcxFile.h
 * Author: fbsagr
 *
 * Created on September 24, 2014, 3:09 PM
 */

#ifndef PCXFILE_H
#define	PCXFILE_H

#include "eastwood/Palette.h"
#include "eastwood/Surface.h"

namespace eastwood {

enum formatVersion {
    PBRUSH_DOS= 0x00,
    V28_PAL = 0x02,
    V28_NOPAL = 0x03,
    PBRUSH_WIN = 0x04,
    V30_STD = 0x05
};

class PcxFile : public Surface
{
    public:
	PcxFile(CCFileClass& fclass);
	~PcxFile();

    private:
    void getPixels(CCFileClass& fclass);
	formatVersion _format;
};

}
#endif	/* PCXFILE_H */

