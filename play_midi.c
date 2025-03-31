#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "mpu401.h"
#include "midi.h"
#include "mem.h"
#include "timer.h"
#include "play_midi.h"

static void turn_off_all_notes(play_midi_t* play_midi)
{
    int i = 0;
    for (; i < 128; i++)
    {
        if (play_midi->trackinfo.notestates[i] != 0)
        {
            int c = 0;
            for (; c < 16; c++)
            {
                if (play_midi->trackinfo.notestates[i] & (1 << c))
                {
                    mpu401_noteoff(c, i);
                }
            }
        }
    }
}

void play_midi_init()
{
  if(mpu401_rst(0x330) == 0)
  {
    mpu401_uart(0x330);
  }
  mem_init(MEM_MALLOC);
}

void play_midi_deinit()
{
    mpu401_rst(0x330);
    mpu401_clear();
}

play_midi_t* play_midi_open(const char* midi_file)
{
    play_midi_t* result = (play_midi_t*) malloc(sizeof(play_midi_t));
    if(result)
    {
        memset(result, 0, sizeof(play_midi_t));

        result->volume = 100;
        if(!loadfile(midi_file, &result->trackinfo, &result->trackpos))
        {
            free(result);
            result = NULL;
        }
    }
    return result;
}

void play_midi_close(play_midi_t* play_midi)
{
    if(play_midi)
    {
        turn_off_all_notes(play_midi);
        mpu401_clear();
        free(play_midi);
    }
}

/* check the event cache for a given event. to reset the cache, issue a single
 * call with trackpos < 0. */
static struct midi_event_t *getnexteventfromcache(struct midi_event_t *eventscache, long trackpos) {
  static unsigned int itemsincache = 0;
  static unsigned int curcachepos = 0;
  struct midi_event_t *res = NULL;
  long nextevent;
  /* if trackpos < 0 then this is only about flushing cache */
  if (trackpos < 0) {
    memset(eventscache, 0, sizeof(*eventscache));
    itemsincache = 0;
    curcachepos = 0;
    return(NULL);
  }
  /* if we have available cache */
  if (itemsincache > 0) {
      curcachepos++;
      curcachepos &= EVENTSCACHEMASK;
      itemsincache--;
      res = &eventscache[curcachepos];
      /* if we have some free time, refill the cache proactively */
      if (res->deltatime > 0) {
        int nextslot, pullres;
        nextslot = curcachepos + itemsincache;
        nextevent = eventscache[nextslot & EVENTSCACHEMASK].next;
        while ((itemsincache < EVENTSCACHESIZE - 1) && (nextevent >= 0)) {
          nextslot++;
          nextslot &= EVENTSCACHEMASK;
          pullres = mem_pull(nextevent, &eventscache[nextslot], sizeof(struct midi_event_t));
          if (pullres != 0) {
            /* printf("pullevent() ERROR: %u (eventid = %ld)\n", pullres, trackpos); */
            return(NULL);
          }
          nextevent = eventscache[nextslot].next;
          itemsincache++;
        }
      }
    } else { /* need to refill the cache NOW */
      int refillcount, pullres;
      nextevent = trackpos;
      curcachepos = 0;
      for (refillcount = 0; refillcount < EVENTSCACHESIZE; refillcount++) {
        pullres = mem_pull(nextevent, &eventscache[refillcount], sizeof(struct midi_event_t));
        if (pullres != 0) {
          return(NULL);
        }
        nextevent = eventscache[refillcount].next;
        itemsincache++;
        if (nextevent < 0) break;
      }
      itemsincache--;
      res = eventscache;
  }
  return(res);
}

bool play_midi_tick(play_midi_t* play_midi)
{
    bool result = true;
    unsigned long t;

    if(play_midi->trackpos >= 0)
    {
        if(play_midi->curevent == NULL)
        {
            play_midi->curevent = getnexteventfromcache(play_midi->eventscache, play_midi->trackpos);
            if(play_midi->curevent)
            {
                play_midi->trackpos = play_midi->curevent->next; 
                if (play_midi->curevent->deltatime > 0)
                {
                    play_midi->nexteventtime += DELTATIME2US(play_midi->curevent->deltatime, play_midi->trackinfo.tempo, play_midi->trackinfo.miditimeunitdiv);
                }
            }          
        }

        mpu401_flush(0x330);

        // is time for next event yet?
        timer_read(&t);

        if(play_midi->curevent)
        {
            if (t >= play_midi->nexteventtime /* || (play_midi->nexteventtime - t > ULONG_MAX / 2) */)
            {
                switch (play_midi->curevent->type) {
                case EVENT_NOTEON:
                    mpu401_noteon(play_midi->curevent->data.note.chan, play_midi->curevent->data.note.note, (play_midi->volume * play_midi->curevent->data.note.velocity) / 100);
                    play_midi->trackinfo.notestates[play_midi->curevent->data.note.note] |= (1 << play_midi->curevent->data.note.chan);
                    break;
                case EVENT_NOTEOFF:
                    mpu401_noteoff(play_midi->curevent->data.note.chan, play_midi->curevent->data.note.note);
                    play_midi->trackinfo.notestates[play_midi->curevent->data.note.note] &= (0xFFFF ^ (1 << play_midi->curevent->data.note.chan));
                    break;
                case EVENT_TEMPO:
                    play_midi->trackinfo.tempo = play_midi->curevent->data.tempoval;
                    break;
                case EVENT_PROGCHAN:
                    play_midi->trackinfo.chanprogs[play_midi->curevent->data.prog.chan] = play_midi->curevent->data.prog.prog;
                    mpu401_setprog(play_midi->curevent->data.prog.chan, play_midi->curevent->data.prog.prog);
                    break;
                case EVENT_PITCH:
                    mpu401_pitchwheel(play_midi->curevent->data.pitch.chan, play_midi->curevent->data.pitch.wheel);
                    break;
                case EVENT_CONTROL:
                    mpu401_controller(play_midi->curevent->data.control.chan, play_midi->curevent->data.control.id, play_midi->curevent->data.control.val);
                    break;
                case EVENT_CHANPRESSURE:
                    mpu401_chanpressure(play_midi->curevent->data.chanpressure.chan, play_midi->curevent->data.chanpressure.pressure);
                    break;
                case EVENT_KEYPRESSURE:
                    mpu401_keypressure(play_midi->curevent->data.keypressure.chan, play_midi->curevent->data.keypressure.note, play_midi->curevent->data.keypressure.pressure);
                        break;
                case EVENT_SYSEX:
                {
                    int i;
                    unsigned short sysexlen = 0;
                    unsigned char *sysexbuff = NULL;

                    // read two bytes from sysexptr so I know how long the thing is
                    mem_pull(play_midi->curevent->data.sysex.sysexptr, &sysexlen, 2);
                    i = sysexlen;
                    if ((i & 1) != 0)
                    {
                        i++;
                    }
                    sysexbuff = (void *)play_midi->wbuff;
                    mem_pull(play_midi->curevent->data.sysex.sysexptr, sysexbuff, i + 2);
                    mpu401_sysex(sysexbuff[2] & 0x0F, sysexbuff + 2, sysexlen);
                    break;
                }
                default:
                    break;
                }          
                play_midi->curevent = NULL;
            }
        }
    }
    else
    {
        result = false;
        play_midi->playing = false;
        turn_off_all_notes(play_midi);
        mpu401_clear();
    }
    return result;
}

void play_midi_start(play_midi_t* play_midi)
{
    play_midi->playing = true;
    play_midi->trackpos = 0;
    timer_read(&play_midi->nexteventtime);
}

void play_midi_stop(play_midi_t* play_midi)
{
    if(play_midi->playing)
    {
        play_midi->playing = false;
        timer_read(&play_midi->stop_time);
        turn_off_all_notes(play_midi);
        mpu401_clear();

    }
}

void play_midi_resume(play_midi_t* play_midi)
{
    play_midi->playing = true;
    play_midi->nexteventtime += timer_time_since(play_midi->stop_time);
}
