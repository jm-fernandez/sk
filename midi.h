/*
 * A simple MIDI parsing library
 *
 * Copyright (C) 2014-2022 Mateusz Viste
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef midi_h_sentinel
#define midi_h_sentinel

#include <stdbool.h>
#include <stdio.h>

/* Rule used to translate an amount of deltatime units into microseconds
 *
 * The MIDI file comes with a "divisor", it is a single 16-bit value in the
 * MIDI header, it defines the number of delta-time units in a beat (quarter
 * note). In other words: it is the "beat length" in delta-time units.
 *
 * Then, there is the tempo value. This is not fixed, can change many times
 * during a song. If not explicitely defined, its defaults to 500'000.
 * Tempo is the "beat length" in micro seconds.
 *
 * Keep in mind that a beat (also called a quarter note) isn't the shortest
 * possible unit of time. The shortest unit is... one delta time unit.
 *
 * ...and for what's it's worth, a beat is divided into 24 MIDI clocks. Yes,
 * the whole thing is insane indeed.
 *
 * The simple rule is (delta * tempo / unitdiv) but due to integer overflow
 * on multiplication with large tempo values, some hacks must be applied.
 *
 * An interesting discussion on the subject can be found here:
 * https://groups.google.com/g/comp.lang.c/c/1Nlc1zRXJqY
 *
 * The optimal solution has been provided by Tim Rentsch the 4th January 2022.
 * Tim's solution is 8x faster than casting the delta * tempo product into an
 * unsigned long long (see tim_mul.c in tests)
 */
#define DELTATIME2US(delta, tempo, unitdiv) ((delta / unitdiv) * tempo + (delta % unitdiv) * (tempo / unitdiv) + (delta % unitdiv) * (tempo % unitdiv) / unitdiv)

#ifdef DBGFILE
/* only needed with DBGFILE so midi_track2events() can use FILE for its log */
#include <stdio.h>
#endif

#define MIDI_OUTOFMEM -10
#define MIDI_EMPTYTRACK -1
#define MIDI_TRACKERROR -2

enum midi_midievents {
  EVENT_NOTEOFF = 0,
  EVENT_NOTEON = 1,
  EVENT_TEMPO = 2,
  EVENT_RAW = 3,
  EVENT_PROGCHAN = 4,
  EVENT_PITCH = 5,
  EVENT_CONTROL = 6,
  EVENT_KEYPRESSURE = 7,
  EVENT_CHANPRESSURE = 8,
  EVENT_SYSEX = 9,
  EVENT_NONE = 100
};

struct midi_event_note_t {
  unsigned char note;
  unsigned char chan;
  unsigned char velocity;
};

struct midi_event_prog_t {
  unsigned char prog;
  unsigned char chan;
};

struct midi_event_pitch_t {
  unsigned short wheel;
  unsigned char chan;
};

struct midi_event_control_t {
  unsigned char id;
  unsigned char chan;
  unsigned char val;
};

struct midi_event_chanpressure_t {
  unsigned char chan;
  unsigned char pressure;
};

struct midi_event_keypressure_t {
  unsigned char chan;
  unsigned char note;
  unsigned char pressure;
};

struct midi_event_sysex_t {
  long sysexptr;
};

struct midi_event_t {
  long next;
  unsigned long deltatime;
  union {
    struct midi_event_note_t note;
    struct midi_event_prog_t prog;
    struct midi_event_pitch_t pitch;
    struct midi_event_control_t control;
    struct midi_event_chanpressure_t chanpressure;
    struct midi_event_keypressure_t keypressure;
    struct midi_event_sysex_t sysex;
    unsigned long tempoval;
  } data;
  enum midi_midievents type;
};

struct trackinfodata {
  unsigned long tempo;
  unsigned long totlen; /* total length, in seconds */
  unsigned long elapsedsec; /* time elapsed (in seconds) */
  unsigned short notestates[128]; /* here I record the state of every note on every channel, to turn all notes OFF in case of program termination */
  unsigned short channelsusage;  /* a bit field indicating what channels are used */
  unsigned char reqpatches[32]; /* bit field of 256 bits indicating what patches (programs) are used, 0-127=melodic ; 128-255=percussion */
  unsigned short miditimeunitdiv;
  unsigned char chanprogs[16];
  int midiformat;
  unsigned short trackscount; /* number of tracks that were found in the file */
};

/* returns number of tracks in midi file on success, neg val otherwise */
int midi_readhdr(FILE* f, int *format, unsigned short *timeunitdiv, unsigned long *tracklist, int maxtracks);

/* parse a track object and returns the id of the first events in the linked list */
long midi_track2events(FILE* f, char *title, int titlemaxlen,
                       char *copyright, int copyrightmaxlen, char *text,
                       int textmaxlen, unsigned short *channelsusage,
                       unsigned long *tracklen, void *reqpatches);

/* merge two MIDI tracks into a single (serialized) one. returns a "pointer"
 * to the unique track. I take care not to allocate/free memory here.
 * All notes are already in RAM after all. totlen is filled with the total
 * time of the merged tracks (in miliseconds). */
long midi_mergetrack(long t0, long t1, unsigned long *totlen, unsigned short timeunitdiv);

bool loadfile(const char* midifile, struct trackinfodata *trackinfo, long *trackpos);

#endif
