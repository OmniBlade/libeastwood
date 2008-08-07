#ifndef EASTWOOD_ANIMATION_H
#define EASTWOOD_ANIMATION_H

#include <SDL.h>

class Animation
{
public:
	Animation();
	~Animation();
	
	SDL_Surface *getFrame();
	void setFrameRate(float FrameRate) {
		if(FrameRate == 0.0) {
			FrameDurationTime = 1;
		} else {
			FrameDurationTime = (int) (1000.0/FrameRate);
		}
	}
	
	void addFrame(SDL_Surface *newFrame, bool SetColorKey = false);

private:
	Uint32 CurFrameStartTime;
	Uint32 FrameDurationTime;
	int curFrame;
	int NumFrames;
	SDL_Surface **Frame;
};

#endif // EASTWOOD_ANIMATION_H