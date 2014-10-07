#ifndef EASTWOOD_WSACNC_H
#define	EASTWOOD_WSACNC_H

#include <vector>

//#include "eastwood/DecodeClass.h"
#include "eastwood/CnCFileClass.h"
#include "eastwood/Palette.h"
#include "eastwood/Surface.h"
#include "eastwood/WsaFile.h"

namespace eastwood {

class WsaCnC //: public DecodeClass
{
public:
	WsaCnC(CCFileClass& fclass, Surface firstFrame = Surface());

	~WsaCnC();

	Surface getSurface(uint16_t frameNumber) const { return _decodedFrames.at(frameNumber); }

	uint16_t size() const throw() { return _decodedFrames.size(); };
	uint32_t getFramesPer1024ms() const throw() { return _framesPer1024ms; };

private:
    void decodeFrames(CCFileClass& fclass);
	std::vector<uint32_t> _frameOffsTable;
	std::vector<Surface> _decodedFrames;

	uint32_t _deltaBufferSize,
		 _framesPer1024ms;
    uint16_t _width,
		 _height;
	Palette _palette;
    uint16_t _xpos,
		 _ypos;
};

}

#endif	/* EASTWOOD_WSACNC_H */

