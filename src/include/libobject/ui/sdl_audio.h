#ifndef __SDL_AUDIO_H__
#define __SDL_AUDIO_H__

#include <libobject/core/obj.h>
#include <libobject/core/string.h>
#include <libobject/ui/audio.h>
#include <SDL2/SDL.h>

typedef struct sdl_audio_s Sdl_Audio;
struct sdl_audio_s{
	Audio parent;

	/*normal methods*/
	int (*construct)(Sdl_Audio *sdl_audio,char *init_str);
	int (*deconstruct)(Sdl_Audio *sdl_audio);
	int (*set)(Sdl_Audio *sdl_audio, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods*/
    int (*init)(Sdl_Audio *audio);
    int (*play)(Sdl_Audio *audio);
    int (*pause)(Sdl_Audio *audio);

	/*attribs*/
	String *path;
    SDL_AudioDeviceID dev_id;
	SDL_AudioSpec wanted_spec;
	SDL_AudioSpec spec;
};

#endif
