
#ifndef EASTWOOD_SHPCNC_H
#define	EASTWOOD_SHPCNC_H

#include <vector>

#include "eastwood/CnCFileClass.h"
#include "eastwood/Palette.h"
#include "eastwood/Surface.h"

namespace eastwood {

class ShpCnC
{
    public:
	ShpCnC(CCFileClass& fclass, Palette palette);
	~ShpCnC();

	/*!
	  This method returns a SDL_Surface containing the nth picture in this shp-File.
	  The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
	  @param	IndexOfFile	specifies which picture to return (zero based)
	  @return	nth picture in this shp-File
	  */
	Surface getSurface(const uint16_t fileIndex) { return _decodedFrames[fileIndex]; }
        Surface& operator[] (uint16_t i) { return _decodedFrames.at(i); }

	uint16_t size() const throw() { return _size; }

    private:
        struct ShpCnCEntry
        {
            uint32_t Offset;
            uint32_t RefOffs;
            uint8_t  Format;
            uint8_t  RefFormat;
        };
        
        void readIndex(CCFileClass& fclass);
        Surface decodeFrame(CCFileClass& fclass, const uint16_t fileIndex);

        std::vector<ShpCnCEntry> _index;
        std::vector<Surface> _decodedFrames;
        uint16_t _size;
        uint16_t _width;
        uint16_t _height;
        Palette _palette;
};

}

#endif	/* EASTWOOD_SHPCNC_H */

