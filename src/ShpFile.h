#ifndef EASTWOOD_SHPFILE_H
#define EASTWOOD_SHPFILE_H

#include "SDL.h"
#include "Decode.h"

#define	TILE_NORMAL		0x00010000
#define	TILE_FLIPH		0x00100000
#define	TILE_FLIPV		0x01000000
#define TILE_ROTATE		0x10000000

#define TILE_GETINDEX(x)	(x & 0x0000FFFF)
#define TILE_GETTYPE(x)		(x & 0xFFFF0000)

struct ShpFileEntry
{
	Uint32 StartOffset;
	Uint32 EndOffset;
};

class Animation;
class SDL_Palette;

class ShpFile : public Decode
{
public:
	ShpFile(unsigned char * bufFiledata, int bufsize, SDL_Palette * palette = NULL);
	~ShpFile();

/*!
	This method returns a SDL_Surface containing the nth picture in this shp-File.
	The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
	@param	IndexOfFile	specifies which picture to return (zero based)
	@return	nth picture in this shp-File
*/
	SDL_Surface *getSurface(Uint32 IndexOfFile);

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
	SDL_Surface *getSurfaceArray(unsigned int tilesX, unsigned int tilesY, ...);
	SDL_Surface *getSurfaceArray(unsigned int tilesX, unsigned int tilesY, Uint32 *tiles);

	/// Returns an animation
/*!
	This method returns a new animation object with all pictures from startindex to endindex
	in it. The returned pointer should be freed with delete if no longer needed. If an error
	occured, NULL is returned.
	@param	startindex	index of the first picture
	@param	endindex		index of the last picture
	@param	SetColorKey	if true, black is set as transparency
	@return	a new animation object or NULL on error
*/
	Animation* getAnimation(unsigned int startindex, unsigned int endindex, bool SetColorKey=true);	

	inline int getNumFiles() {return (int) NumFiles;};

protected:
	Uint16 NumFiles;

private:
	void readIndex();

	ShpFileEntry * Index;
	unsigned char* Filedata;
	Uint32 ShpFilesize;
	SDL_Palette * m_palette;
};


#endif // EASTWOOD_SHPFILE_H