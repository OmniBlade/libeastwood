
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


	/*!
	  This method returns a SDL_Surface containing an array of pictures from this shp-File.
	  All pictures must be of the same size. tilesX/tilesY specifies how many pictures are in this row/column.
	  Afterwards there must be tilesX*tilesY many parameters. Every parameter specifies which picture 
	  of this shp-File should be used. This indices must be ORed with a parameter specifing hwo they should
	  be in the result surface. There are 4 modes and you must OR exactly one:
	  - TILE_NORMAL	Normal
	  - TILE_FLIPH	mirrored horizontally
	  - TILE_FLIPV	mirrored vertically
	  - TILE_ROTATE	Rotated by 180 degress

	  Example:
	  @code
	  picture = myShpfile->getSurfaceArray(4,1, TILE_NORMAL | 20, TILE_FLIPH | 23, TILE_ROTATE | 67, TILE_NORMAL | 68);
	  @endcode
	  This example would create a surface with four pictures in it. From the left to the right there are
	  picture 20,23,67 and 68. picture 23 is mirrored horizontally, 67 is rotated.<br><br>
	  The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
	  @param	tilesX	how many pictures in one row
	  @param	tilesY	how many pictures in one column
	  @return	picture in this shp-File containing all specified pictures
	  */
	uint16_t size() const throw() { return _size; }

    private:
        struct ShpCnCEntry
        {
            uint32_t Offset;
            uint8_t  Format;
            uint32_t RefOffs;
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

