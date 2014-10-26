#ifndef EASTWOOD_WSAFILE_H
#define EASTWOOD_WSAFILE_H
#include <vector>
#include <iostream>

#include "eastwood/BaseImageSequence.h"

namespace eastwood {

class WsaFile : public BaseImageSequence
{
public:
	WsaFile(std::istream &stream, Palette palette = Palette(0));

	~WsaFile();

	uint32_t getFramesPer1024ms() const throw() { return _framesPer1024ms; };

private:
	void decodeFrames(std::istream& stream);
	std::vector<uint32_t> _frameOffsTable;
        
        unsigned int _width;
        unsigned int _height;
	uint32_t _deltaBufferSize,
		 _framesPer1024ms;
                 
};

}
#endif // EASTWOOD_WSAFILE_H
