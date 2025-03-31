#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "write_effect.h"
#include "render.h"
#include "sprite.h"
#include "timer.h"
#include "util.h"

typedef struct write_effect_t_ {
  effect_t effect;
  bool clean;
  const font_t* font;
  int size;
  const char* text;
  int x;
  int y;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long time;
  bool finished;
} write_effect_t;

static void show_text(const font_t* font, const char* txt, int len, int x_pos, int y_pos, int size)
{
  int i = 0;
  for(; i < len; ++i)
  {
    if(txt[i] == ' ')
    {
        x_pos += size;
    }
    else
    {
      const sprite_t* letter = font_get_letter(font, txt[i]);
      if(letter)
      {
        const unsigned int letter_width = sprite_width(letter);
        const unsigned int letter_height = sprite_height(letter);
        square_t square = {0};

        square.x = x_pos;
        square.y = y_pos;
        square.sy = size;
        square.sx = size == letter_height ? letter_width : (int) ((double) size / letter_height * letter_width);

        render_sprite(letter, &square, false);
        x_pos += square.sx + square.sy / 10;
      }
    }
  }
}

void write_effect_start(effect_t* effect)
{
  write_effect_t* write_effect = (write_effect_t*)effect;
  timer_read(&write_effect->start_time);
  write_effect->finished = false;
  if(write_effect->clean)
  {
    render_clean();
  }
}

static void write_effect_stop(effect_t* effect)
{
  write_effect_t* write_effect = (write_effect_t*)effect;
  timer_read(&write_effect->stop_time);
}

static void write_effect_resume(effect_t* effect)
{
  write_effect_t* write_effect = (write_effect_t*)effect;
  const unsigned long elapsed = timer_time_since(write_effect->stop_time);
  write_effect->start_time += elapsed;
}

void write_effect_step(effect_t* effect)
{
  write_effect_t* write_effect = (write_effect_t*)effect;
  const unsigned long elapsed = timer_time_since(write_effect->start_time);
  const int max_letter_count = strlen(write_effect->text);
  const double relative_time = (double) elapsed / write_effect->time;
  const double letter_count_dbl = relative_time * max_letter_count;
  const int letter_count = min((int) letter_count_dbl, max_letter_count); 

  if(write_effect->clean)
  {
    render_clean();
  }

  if(letter_count > 0)
  {
    show_text(
      write_effect->font, 
      write_effect->text, 
      letter_count, 
      write_effect->x, 
      write_effect->y,
      write_effect->size);
  }

  write_effect->finished = (letter_count == max_letter_count);
}

bool write_effect_finished(effect_t* effect)
{
  const write_effect_t* write_effect = (const write_effect_t*)effect;
  return write_effect->finished;
}

void write_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_write_effect(const font_t* font, const char* text, int x, int y, int size, unsigned long time, bool clean)
{
  write_effect_t* effect = (write_effect_t*) malloc(sizeof(write_effect_t));
  if(effect)
  {
    effect->clean = clean;
    effect->start_time = 0U;
    effect->size = size;
    effect->font = font;
    effect->text = text;
    effect->x = x;
    effect->y = y;
    effect->time = time;
    effect->finished = false;

    effect->effect.finished = write_effect_finished;
    effect->effect.free = write_effect_free;
    effect->effect.step = write_effect_step;
    effect->effect.start = write_effect_start;
    effect->effect.stop = write_effect_stop;
    effect->effect.resume = write_effect_resume;
  }
  return effect ? &(effect->effect) : NULL;
}
