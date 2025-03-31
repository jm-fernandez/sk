#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include "play_wav.h"
#include "audio.h"
#include "aufile.h"
#include "log.h"

static const char* sound_names[] = {
    "lucha",
    "punch",
};

static struct au_file* wav_files[PLAY_WAV_COUNT] = {0};

static int play_wav_callback(void *buf, int size, void *cls)
{
	int rd = 0;
    struct au_file* file = (struct au_file*) cls;
	if((rd = au_read(file, buf, size)) <= 0)
    {
		rd = 0;
	}
	return rd;
}

bool play_wav_init()
{
    bool result = audio_init() != -1;
    if(result)
    {
        char file_path[256];
        int i  = 0;
        audio_setvolume(AUDIO_MASTER, 255);
        audio_setvolume(AUDIO_PCM, 255);

        for(; i < PLAY_WAV_COUNT; ++i)
        {
            sprintf(file_path, "assets\\sounds\\%s.wav", sound_names[i]);
            wav_files[i] = au_open(file_path);
            if(wav_files[i] == NULL)
            {
                char error[256];
                sprintf(error, "Error loading %s", file_path);
                log_record(error);
                break;
            }
        }
        if(i < PLAY_WAV_COUNT)
        {
            play_wav_deinit();
            result = false;
        }
    }
    return result;
}

void play_wav_deinit()
{
    int i = 0;

    play_wav_stop();
    for(; i < PLAY_WAV_COUNT; ++i)
    {
        if( wav_files[i] != NULL)
        {
            au_close(wav_files[i]);
            wav_files[i] = NULL;
        }
    }
}

void play_wav_start(int index)
{
    if(play_wav_busy())
    {
        audio_stop();
    }
    au_reset(wav_files[index]);
    audio_set_callback(play_wav_callback, wav_files[index]);
    audio_play(wav_files[index]->rate, wav_files[index]->bits, wav_files[index]->chan);
}

void play_wav_stop()
{
    audio_stop();
}

void play_wav_resume()
{
    audio_resume();
}

bool play_wav_busy()
{
    return audio_isplaying() != 0;
}
