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

#include <string.h>
#include <stdio.h>

#include "bitfield.h"
#include "mem.h"
#include "midi.h"   /* include self for control */

#define MAXTRACKS 64


#define BSWAPL(x) ((((unsigned long)(x) & 0x000000FFul) << 24) | (((unsigned long)(x) & 0x0000FF00ul) << 8) | (((unsigned long)(x) & 0x00FF0000ul) >> 8) | (((unsigned long)(x) & 0xFF000000ul) >> 24))

extern unsigned char wbuff[];

/* PRIVATE ROUTINES USED FOR INTERNAL PROCESSING ONLY */

/* fetch a variable length quantity value from a given offset. returns number of bytes read */
static int midi_fetch_variablelen_fromfile(FILE *f, unsigned long *result) {
  unsigned char bytebuff;
  int offset = 0;
  *result = 0;
  for (;;) {
    fread(&bytebuff, sizeof(bytebuff), 1, f);
    *result <<= 7;
    *result |= (bytebuff & 127);
    if ((bytebuff & 128) == 0) break;
  }
  return(offset);
}


/* reads a MIDI file and computes a map of chunks (ie a list of offsets) */
static int midi_gettrackmap(FILE* f, unsigned long *tracklist, int maxchunks) {
  short i;
  unsigned long ulvar;
  for (i = 0; i < maxchunks; i++) {
    /* read and validate chunk's id */
    if(fread(&ulvar, sizeof(ulvar), 1, f) != 1) {
      break;
    }

    if (ulvar != 0x6b72544dL) return(-1); /* if header != "MTrk" */
    /* compute the track's byte length */
    if(fread(&ulvar, sizeof(ulvar), 1, f) != 1) {
      break;
    }
    ulvar = BSWAPL(ulvar);
    /* remember chunk data offset */
    tracklist[i] = ftell(f);
    /* skip to next chunk */
    fseek(f, ulvar, SEEK_CUR);
  }
  return(i);
}


/* PUBLIC INTERFACE */

int midi_readhdr(FILE* f, int *format, unsigned short *timeunitdiv, unsigned long *tracklist, int maxtracks) {
  unsigned short tracks;
  /*
   * Here's an example of a complete MThd chunk:
   *  4D 54 68 64     MThd ID
   *  00 00 00 06     Length of the MThd chunk is always 6
   *  00 01           The Format type is 1
   *  00 02           There are 2 MTrk chunks in this file
   *  E7 28           Each increment of delta-time represents a millisecond
   */

  /* test for RMID header */
  /* a RMID file starts with RIFFxxxxRMID (xxxx being the data size) followed
   * by the word 'data' followed by a 32-bit data size. */
  /* read first 14 bytes - if unable, return an error */
  if(fread(wbuff, 1, 14, f)!= 14) {
    return -8;
  }

  /* if no RMID header, then assume it's normal MIDI */
  if ((wbuff[0] == 'R') && (wbuff[1] == 'I')  && (wbuff[2] == 'F') && (wbuff[3] == 'F')
   && (wbuff[8] == 'R') && (wbuff[9] == 'M') && (wbuff[10] == 'I') && (wbuff[11] == 'D')) {
    /* skip 6 bytes and there we should have our MThd MIDI header */
    fseek(f, 6, SEEK_CUR);
    if(fread(wbuff, 1, 14, f) != 14)
    {
      return -7;
    }
  }

  /* check id (MThd) and len (must be exactly 6 bytes) */
  if ((((unsigned long *)wbuff)[0] != 0x6468544dL) || (wbuff[4] != 0) || (wbuff[5] != 0) || (wbuff[6] != 0) || (wbuff[7] != 6)) { /* 0x6468544dL == "MThd" */
    return(-6);
  }

  if (wbuff[8] != 0) return(-5); /* format is 1 or 2 so 1st digit must be 0 */
  *format = wbuff[9];

  tracks = wbuff[10];
  tracks <<= 8;
  tracks |= wbuff[11];
  /* midi_gettrackmap() should not try reading more tracks than declared in
   * the header - some MIDI/RMI files may be trailed with some extra stuff */
  if (tracks < maxtracks) maxtracks = tracks;

  *timeunitdiv = wbuff[12];
  *timeunitdiv <<= 8;
  *timeunitdiv |= wbuff[13];

  /* timeunitdiv must be a positive number */
  if (*timeunitdiv < 1) return(-3);

  /* default tempo -> quarter note (1 beat) == 500'000 microseconds (0.5s), ie 120 bpm.
     a delta time unit is therefore (0.5s / DIV) long. */

  if (*format > 2) return(-2);

  /* read the tracks map and return number of tracks */
  return(midi_gettrackmap(f, tracklist, maxtracks));
}


/* returns a negative value on error, 0 on success, 1 on end of track */
static int ld_meta(struct midi_event_t *event, FILE* f, unsigned long *tracklen, char *title, int titlemaxlen, char *copyright, int copyrightmaxlen, char *text, int textmaxlen) {
  unsigned long metalen;
  unsigned long i;
  int result = 0;
  unsigned char subtype;
  fread(&subtype, sizeof(subtype), 1, f);
  midi_fetch_variablelen_fromfile(f, &metalen);
  switch (subtype) {
    case 1: /* text or marker - often used to describe the file... */
    case 6: /* marker */
      i = 0;
      if ((text != NULL) && (text[0] == 0) && (textmaxlen > 3)) { /* title might be NULL */
        for (; i < metalen; i++) {
          if (i+1 >= textmaxlen) break; /* avoid overflow */
          fread(text + i, 1, 1, f);
        }
        text[i] = 0;
        /* recompute the available maxlen */
        text += i;
        textmaxlen -= i;
        /* add a LF trailer, just in case we'd like to append more data */
        if (textmaxlen > 2) {
          *text = '\n';
          text++;
          *text = 0;
          textmaxlen--;
        }
      }
      /* skip the rest, if we had to truncate the string */
      fseek(f, metalen - i, SEEK_CUR);
      break;
    case 2: /* copyright notice */
      i = 0;
      if ((copyright != NULL) && (copyright[0] == 0)) { /* take care, copyright might be NULL */
        for (; i < metalen; i++) {
          if (i+1 >= copyrightmaxlen) break; /* avoid overflow */
          fread(copyright + i, 1, 1, f);
        }
        copyright[i] = 0;
      }
      fseek(f, metalen-i, SEEK_CUR);
      break;
    case 3: /* track name */
      i = 0;
      if (title != NULL) { /* title might be NULL */
        for (; i < metalen; i++) {
          if (i+1 >= titlemaxlen) break; /* avoid overflow */
          fread(title +i, 1, 1, f);
        }
        title[i] = 0;
      }
      fseek(f, metalen-i, SEEK_CUR);
      break;
    case 4: /* instrument name */
      fseek(f, metalen, SEEK_CUR);
      break;
    case 5: /* lyric */
      fseek(f, metalen, SEEK_CUR);
      break;
    case 0x21:  /* MIDI port -- no support for multi-MIDI files, I just ignore it */
      fseek(f, metalen, SEEK_CUR);
      break;
    case 0x2F: /* end of track */
      result = 1;
      break;
    case 0x51:  /* set tempo */
      if (metalen != 3) {
        return(-1);
      } else {
        unsigned char b[3];
        event->type = EVENT_TEMPO;
        fread(b, sizeof(b), 1, f);
        event->data.tempoval = b[0];
        event->data.tempoval <<= 8;
        event->data.tempoval |= b[1];
        event->data.tempoval <<= 8;
        event->data.tempoval |= b[2];
      }
      break;
    case 0x54:  /* SMPTE offset -> since I expect only format 0/1 files, I ignore this because I want to start playing asap anyway */
      fseek(f, metalen, SEEK_CUR);
      break;
    case 0x58:  /* Time signature */
      if (metalen != 4) {
        return(-1);
      } else {
        fseek(f, metalen, SEEK_CUR);
      }
      break;
    case 0x59:  /* key signature */
      if (metalen != 2) {
        return(-1);
      } else {
        fseek(f, metalen, SEEK_CUR);
      }
      break;
    case 0x7F:  /* proprietary event -> this is non-standard stuff, I ignore it */
      fseek(f, metalen, SEEK_CUR);
      break;
    default:
      fseek(f, metalen, SEEK_CUR);
      break;
  }
  return(result);
}


/* returns a negative value on error, 0 on success, 1 on end of track */
static int ld_sysex(struct midi_event_t *event, FILE *f, unsigned char statusbyte, unsigned long *tracklen) {
  unsigned long sysexlen;
  int sysexleneven; /* can be int, guaranteed to be less than 4K */
  unsigned char *sysexbuff;
  midi_fetch_variablelen_fromfile(f, &sysexlen); /* get length */
  sysexlen += 1; /* add one byte for the status byte that is not counted, but that we will add to the top of the buffer later */
  if (sysexlen > 4096) { /* skip SYSEX events that are more than 4K big */
    fseek(f, sysexlen, SEEK_CUR);
    return(0);
  }
  /* read the sysex string */
  sysexleneven = sysexlen + 2; /* add two bytes for the sysex length that I will add in front of the actual sysex string */
  if ((sysexleneven & 1) != 0) sysexleneven++; /* make sysexleneven an even number (XMS moves MUST occur on even numbers of bytes) */
  sysexbuff = wbuff;
  event->type = EVENT_SYSEX;

  ((unsigned short *)sysexbuff)[0] = sysexlen;
  sysexbuff[2] = statusbyte; /* I store the entire sysex string in memory */
  fread(sysexbuff + 3, sysexlen - 1, 1, f);
  event->data.sysex.sysexptr = mem_alloc(sysexleneven);
  if (event->data.sysex.sysexptr >= 0) {
    mem_push(sysexbuff, event->data.sysex.sysexptr, sysexleneven);
  } else {
    event->type = EVENT_NONE;
    return(MIDI_OUTOFMEM);
  }
  return(0);
}

static int ld_note(struct midi_event_t *event, FILE* f, unsigned char statusbyte, unsigned long *tracklen, unsigned short *channelsusage, void *reqpatches) {
  unsigned char ubuff[2]; /* micro buffer for loading data */
  switch (statusbyte & 0xF0) { /* I care only about NoteOn/NoteOff events */
    case 0x80:  /* Note OFF */
      fread(ubuff, sizeof(ubuff), 1, f);
      event->type = EVENT_NOTEOFF;
      event->data.note.chan = statusbyte & 0x0F;
      event->data.note.note = ubuff[0] & 127; /* a note must be in range 0..127 */
      event->data.note.velocity = ubuff[1];
      break;
    case 0x90:  /* Note ON */
      fread(ubuff, sizeof(ubuff), 1, f);
      event->type = EVENT_NOTEON;
      event->data.note.chan = statusbyte & 0x0F;
      event->data.note.note = ubuff[0] & 127;
      event->data.note.velocity = ubuff[1];
      if (event->data.note.velocity == 0) {
        event->type = EVENT_NOTEOFF; /* if no velocity, it's in fact a note OFF */
      } else {
        *channelsusage |= (1 << event->data.note.chan); /* update the channel usage flags */
      }
      /* if it's percussion, mark the required patch */
      if (event->data.note.chan == 9) BIT_SET(reqpatches, event->data.note.note | 128);
      break;
    case 0xA0:  /* key after-touch */
        fread(ubuff, sizeof(ubuff), 1, f);
      event->type = EVENT_KEYPRESSURE;
      event->data.keypressure.chan = statusbyte & 0x0F;
      event->data.keypressure.note = ubuff[0];
      event->data.keypressure.pressure = ubuff[1];
      break;
    case 0xB0:  /* control change */
        fread(ubuff, sizeof(ubuff), 1, f);
      event->type = EVENT_CONTROL;
      event->data.control.chan = statusbyte & 0x0F;
      event->data.control.id = ubuff[0];
      event->data.control.val = ubuff[1];
      break;
    case 0xC0:  /* program (patch) change */
      fread(ubuff, 1, 1, f);
      event->type = EVENT_PROGCHAN;
      event->data.prog.chan = statusbyte & 0x0F;
      event->data.prog.prog = ubuff[0] & 127;
      BIT_SET(reqpatches, event->data.prog.prog);
      break;
    case 0xD0:  /* channel after-touch (aka "channel pressure") */
        fread(ubuff, 1, 1, f);
      event->type = EVENT_CHANPRESSURE;
      event->data.chanpressure.chan = statusbyte & 0x0F;
      event->data.chanpressure.pressure = ubuff[0];
      break;
    case 0xE0:  /* pitch wheel change */
      fread(ubuff, sizeof(ubuff), 1, f);
      event->type = EVENT_PITCH;
      event->data.pitch.chan = statusbyte & 0x0F;
      event->data.pitch.wheel = ubuff[1];
      event->data.pitch.wheel <<= 7;
      event->data.pitch.wheel |= ubuff[0];
      break;
    default:
      return(-1);
      break;
  }
  return(0);
}


/* parse a track object and returns the id of the first events in the linked
 * list. channelsusage contains 16 flags indicating what channels are used.
 * titlemaxlen and copyrightmaxlen are the maximum lengths of the strings,
 * including the NULL terminator.
 * returns MIDI_EMPTYTRACK if no event found in the track
 * returns MIDI_TRACKERROR if the track is corrupted
 * returns MIDI_OUTOFMEM if failed to store events in memory */
long midi_track2events(FILE* f, char *title, int titlemaxlen, char *copyright, int copyrightmaxlen, char *text, int textmaxlen, unsigned short *channelsusage, unsigned long *tracklen, void *reqpatches) {
  unsigned long deltatime;
  unsigned char statusbyte = 0;
  struct midi_event_t event;
  long result = MIDI_EMPTYTRACK;
  unsigned long ignoreddeltas = 0;

  /* zero out title and copyright strings, if provided */
  if (titlemaxlen > 0) title[0] = 0;
  if (copyrightmaxlen > 0) copyright[0] = 0;
  if (textmaxlen > 0) text[0] = 0;

  *tracklen = 0;

  for (;;) {
    int r;
    unsigned char bytebuff;
    /* read the delta time first - variable length */
    midi_fetch_variablelen_fromfile(f, &deltatime);
    *tracklen += deltatime;
    /* check the type of the event */
    /* if it's a byte with MSB set, we are dealing with running status (so it's same status as last time */
    if(fread(&bytebuff, 1, 1, f) == 0) {
      return MIDI_TRACKERROR;
    }
    if ((bytebuff & 128) != 0) {
      statusbyte = bytebuff;
    } else { /* get back one byte */
      fseek(f, -1, SEEK_CUR);
    }
    event.type = EVENT_NONE;
    event.deltatime = deltatime;
    event.next = -1;
    if (statusbyte == 0xFF) { /* META event */
      r = ld_meta(&event, f, tracklen, title, titlemaxlen, copyright, copyrightmaxlen, text, textmaxlen);
      if (r < 0) {
        return(MIDI_TRACKERROR);
      }
      if (r == 1) break; /* end of track */
    } else if ((statusbyte >= 0xF0) && (statusbyte <= 0xF7)) { /* SYSEX event */
      r = ld_sysex(&event, f, statusbyte, tracklen);
      if (r != 0) {
        return(MIDI_TRACKERROR);
      } 
    } else if ((statusbyte >= 0x80) && (statusbyte <= 0xEF)) { /* else it's a note-related command */
      r = ld_note(&event, f, statusbyte, tracklen, channelsusage, reqpatches);
      if (r != 0) {
        return(MIDI_TRACKERROR);
      }
    } else { /* else it's an error - free memory we allocated and return NULL */
      return(MIDI_TRACKERROR);
    }
    /* add the event to the queue */
    if (event.type == EVENT_NONE) {
      ignoreddeltas += event.deltatime;
    } else {
      int pusheventres;
      event.deltatime += ignoreddeltas; /* add any previously ignored delta times */
      ignoreddeltas = 0;
      /* add the event to the queue */
      if (result == MIDI_EMPTYTRACK) { /* this is the first event in the queue */
        pusheventres = pusheventqueue(&event, &result);
      } else {
        pusheventres = pusheventqueue(&event, NULL);
      }
      if (pusheventres != 0) {
        return(MIDI_OUTOFMEM);
      }
    }
  }
  if (result >= 0) {
    if (pusheventqueue(NULL, NULL) != 0) {
      return(MIDI_OUTOFMEM); /* flush last event in buffer to memory */
    }
  }
  return(result);
}


/* merge two MIDI tracks into a single (serialized) one. returns a "pointer"
 * to the unique track. I take care not to allocate/free memory here.
 * All notes are already in RAM after all. totlen is filled with the total
 * time of the merged tracks (in seconds). */
long midi_mergetrack(long t0, long t1, unsigned long *totlen, unsigned short timeunitdiv) {
  long res = -1, lasteventid = -1, selectedid;
  int selected;
  unsigned long curtempo = 500000l, utotlen = 0;
  struct midi_event_t event[2], lastevent;

  if (totlen != NULL) *totlen = 0;
  /* fetch first events for both tracks */
  if (t0 >= 0) mem_pull(t0, &event[0], sizeof(struct midi_event_t));
  if (t1 >= 0) mem_pull(t1, &event[1], sizeof(struct midi_event_t));
  /* start looping */
  while ((t0 >= 0) || (t1 >= 0)) {
    /* compare both tracks, and select the soonest one */
    if (t0 >= 0) {
      if ((t1 >= 0) && (event[1].deltatime < event[0].deltatime)) {
        selected = 1;
        selectedid = t1;
      } else {
        selected = 0;
        selectedid = t0;
      }
    } else {
      selected = 1;
      selectedid = t1;
    }
    /* on first iteration, make sure to assign a result */
    if (lasteventid < 0) {
      res = selectedid;
    } else if (lastevent.next != selectedid) {                        /* otherwise attach selected */
      lastevent.next = selectedid;                                    /* track to last note, and   */
      mem_push(&lastevent, lasteventid, sizeof(struct midi_event_t)); /* update last pointer (if   */
    }                                                                 /* not good already)         */
    /* save the last event into buffer for later, and remember its id */
    lasteventid = selectedid;
    memcpy(&lastevent, &(event[selected]), sizeof(struct midi_event_t));
    /* increment timer */
    if ((totlen != NULL) && (event[selected].deltatime != 0)) {
      utotlen += DELTATIME2US(event[selected].deltatime, curtempo, timeunitdiv);
      while (utotlen >= 1000000lu) {
        utotlen -= 1000000lu;
        *totlen += 1;
      }
    }
    if (event[selected].type == EVENT_TEMPO) curtempo = event[selected].data.tempoval;
    /* decrement timer on the non-selected track, and synch it to xms if needed */
    /* then move along on the selected track, and fetch it from xms */
    if (selected == 0) {
      if ((t1 >= 0) && (event[0].deltatime != 0)) {
        event[1].deltatime -= event[0].deltatime;
        mem_push(&event[1], t1, sizeof(struct midi_event_t));
      }
      t0 = event[0].next;
      if (t0 >= 0) mem_pull(t0, &event[0], sizeof(struct midi_event_t));
    } else { /* selected == 1 */
      if ((t0 >= 0) && (event[1].deltatime != 0)) {
        event[0].deltatime -= event[1].deltatime;
        mem_push(&event[0], t0, sizeof(struct midi_event_t));
      }
      t1 = event[1].next;
      if (t1 >= 0) mem_pull(t1, &event[1], sizeof(struct midi_event_t));
    }
  }
  return(res);
}

static bool loadfile_midi(FILE* f, struct trackinfodata *trackinfo, long *trackpos) {
  static unsigned long trackmap[MAXTRACKS];
  int miditracks;
  int i;
  long newtrack;
  char text[256];

  *trackpos = -1;

  miditracks = midi_readhdr(f, &(trackinfo->midiformat), &(trackinfo->miditimeunitdiv), trackmap, MAXTRACKS);
  if (miditracks < 1) {
    return false;
  }

  trackinfo->trackscount = miditracks;
  if ((trackinfo->midiformat != 0) && (trackinfo->midiformat != 1)) {
    return false;
  }

  if (miditracks > MAXTRACKS) {
    return false;
  }

  for (i = 0; i < miditracks; i++) {
    unsigned long tracklen;

    fseek(f, trackmap[i], SEEK_SET);
    newtrack = midi_track2events(f, NULL, 0, NULL, 0,
                                  NULL, 0, &(trackinfo->channelsusage),
                                  &tracklen, trackinfo->reqpatches);

    /* look for error conditions */
    if (newtrack == MIDI_OUTOFMEM || newtrack == MIDI_TRACKERROR) {
      return false;
    }

    /* merge the track now */
    if (newtrack >= 0) {
      *trackpos = midi_mergetrack(*trackpos, newtrack, &(trackinfo->totlen), trackinfo->miditimeunitdiv);
    }
  }
  return true;
}

bool loadfile(const char* midifile, struct trackinfodata *trackinfo, long *trackpos) {
  unsigned char hdr[16];
  unsigned short sel;
  bool res = false;

  FILE* file = fopen(midifile, "rb");
  if(!file) {
    return false;
  }

  /* read first few bytes of the file to detect its format, and rewind */
  if(fread(hdr, 1, 16, file) != 16) {
    fclose(file);
    return false;
  }

  // if it is not a midi
  if((hdr[0] != 'M') || (hdr[1] != 'T') || (hdr[2] != 'h') || (hdr[3] != 'd')) {
    fclose(file);
    return false;
  }

  fseek(file, 0, SEEK_SET);
  res = loadfile_midi(file, trackinfo, trackpos);
  fclose(file);

  return(res);
}
