#ifndef PLAY_WAV_H__
#define PLAY_WAV_H__

#include "aufile.h"

#define PLAY_WAV_FIGHT  0
#define PLAY_WAV_HIT    1
#define PLAY_WAV_COUNT  2

bool play_wav_init();
void play_wav_deinit();

void play_wav_start(int wav_file);
void play_wav_stop();
void play_wav_resume();
bool play_wav_busy();

#endif // PLAY_WAV_H__
