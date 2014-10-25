#ifndef EASTWOOD_WSAFILE_H
#define EASTWOOD_WSAFILE_H
#include <vector>
#include <iostream>

#include "eastwood/Decode.h"
#include "eastwood/BaseImage.h"

namespace eastwood {

class WsaFile
{
public:
	WsaFile(std::istream &stream, Palette palette = Palette());

	~WsaFile();

	Surface getSurface(uint16_t frameNumber);

	uint16_t size() const throw() { return _decodedFrames.size(); };
	uint32_t getFramesPer1024ms() const throw() { return _framesPer1024ms; };

private:
	void decodeFrames(std::istream& stream);
	std::vector<uint32_t> _frameOffsTable;
	std::vector<BaseImage> _decodedFrames;
        Palette _palette;
        
        unsigned int _width;
        unsigned int _height;
	uint32_t _deltaBufferSize,
		 _framesPer1024ms;
                 
};

}
#endif // EASTWOOD_WSAFILE_H
