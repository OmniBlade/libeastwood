#ifndef EASTWOOD_WSAFILE_H
#define EASTWOOD_WSAFILE_H
#include <vector>

#include "eastwood/Decode.h"

namespace eastwood {

class WsaFile
{
public:
	WsaFile(std::istream &stream, Palette palette = Palette(),
                Surface firstFrame = Surface());

	~WsaFile();

	Surface getSurface(uint16_t frameNumber) const { return _decodedFrames.at(frameNumber); }

	uint16_t size() const throw() { return _decodedFrames.size(); };
	uint32_t getFramesPer1024ms() const throw() { return _framesPer1024ms; };

private:
	void decodeFrames(std::istream &stream);
	std::vector<uint32_t> _frameOffsTable;
	std::vector<Surface> _decodedFrames;
        Palette _palette;
        
        uint32_t _width;
        uint32_t _height;
	uint32_t _deltaBufferSize,
		 _framesPer1024ms;
};

}
#endif // EASTWOOD_WSAFILE_H
