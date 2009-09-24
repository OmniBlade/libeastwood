#ifndef EASTWOOD_SDL_MIXER_SOUND_H
#define EASTWOOD_SDL_MIXER_SOUND_H

#include <SDL_mixer.h>

#include "eastwood/Sound.h"

namespace eastwood { namespace SDL { namespace Mixer {

class Sound : public eastwood::Sound, public Mix_Chunk
{
    public:
	Sound();
	Sound(uint32_t size, uint8_t *buffer, uint32_t frequency, uint8_t channels, AudioFormat format);
	Sound(uint32_t size, uint8_t *buffer);
	Sound(const eastwood::Sound &sound);
	virtual ~Sound();

	Sound getResampled(Interpolator interpolator = I_LINEAR);

    private:
	Mix_Chunk *_sound;

};

}}}
#endif // EASTWOOD_SDL_MIXER_SOUND_H