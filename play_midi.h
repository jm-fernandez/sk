#ifndef PLAY_MIDI_H__
#define PLAY_MIDI_H__

#include <stdbool.h>

#include "midi.h"

#define AUX_BUFFER_SIZE     8192 
#define EVENTSCACHESIZE     64
#define EVENTSCACHEMASK     63

typedef struct play_midi_t_
{
  bool playing;
  struct trackinfodata trackinfo;
  struct midi_event_t eventscache[EVENTSCACHESIZE];  
  unsigned char wbuff[AUX_BUFFER_SIZE];   
  struct midi_event_t *curevent;
  long trackpos;
  unsigned long nexteventtime;
  unsigned long stop_time;
  int volume;
}play_midi_t;


void play_midi_init();
void play_midi_deinit();

play_midi_t* play_midi_open(const char* midi_file);
void play_midi_close(play_midi_t* play_midi);

bool play_midi_tick(play_midi_t* play_midi);

void play_midi_start(play_midi_t* play_midi);
void play_midi_stop(play_midi_t* play_midi);
void play_midi_resume(play_midi_t* play_midi);

static bool play_midi_is_playing(play_midi_t* play_midi)
{
    return play_midi->playing;
}

#endif // PLAY_MIDI_H__
