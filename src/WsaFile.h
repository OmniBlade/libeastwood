#ifndef EASTWOOD_WSAFILE_H
#define EASTWOOD_WSAFILE_H
#include <istream>
#include <vector>

#include "Decode.h"


class WsaFile : public Decode
{
public:
	WsaFile(std::istream &stream, SDL_Palette *palette,
                SDL_Surface *lastframe = NULL, float setFps = 0 );

	~WsaFile();

	SDL_Surface *getSurface(uint32_t FrameNumber);

	inline uint32_t getNumFrames() { return (uint32_t) _numFrames; };
	inline uint32_t getFramesPer1024ms() { return _framesPer1024ms; };
	inline float getFPS() { return _fps; }

private:
	void decodeFrames();
	std::vector<uint32_t> _frameOffsTable;
	std::vector<uint8_t> _decodedFrames;

	uint16_t _numFrames,
		 _flags,
		 _deltaBufferSize;

	uint32_t _framesPer1024ms;
	float _fps;
};

#endif // EASTWOOD_WSAFILE_H
