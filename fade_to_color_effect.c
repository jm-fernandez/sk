#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "fade_to_color_effect.h"
#include "palette.h"
#include "render.h"
#include "timer.h"

typedef struct fade_to_color_effect_t_ {
  effect_t effect;
  const palette_t* palette;
  palette_entry_t target;
  int start_color;
  int end_color;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long time;
  double step;
  bool finished;
}fade_to_color_effect_t;


void fade_to_color_effect_start(effect_t* effect)
{
  fade_to_color_effect_t* fade_to_color_effect = (fade_to_color_effect_t*)effect;
   const palette_t* palette = fade_to_color_effect->palette;
  unsigned long biggest_difference = 0;
  int i = fade_to_color_effect->start_color;

  for(; i < fade_to_color_effect->end_color; ++i)
  {
    const int red_difference = abs(palette->colors[i].red - fade_to_color_effect->target.red);
    const int green_difference = abs(palette->colors[i].green - fade_to_color_effect->target.green);
    const int blue_difference = abs(palette->colors[i].blue - fade_to_color_effect->target.blue);

    biggest_difference = max(biggest_difference, red_difference);
    biggest_difference = max(biggest_difference, green_difference);
    biggest_difference = max(biggest_difference, blue_difference);
  }
  render_set_palette(palette);
  fade_to_color_effect->step = (double) biggest_difference / fade_to_color_effect->time;
  timer_read(&(fade_to_color_effect->start_time));
  fade_to_color_effect->finished = false;
}

void fade_to_color_effect_stop(effect_t* effect)
{
  fade_to_color_effect_t* fade_to_color_effect = (fade_to_color_effect_t*)effect;
  timer_read(&(fade_to_color_effect->stop_time));
}

void fade_to_color_effect_resume(effect_t* effect)
{
  fade_to_color_effect_t* fade_to_color_effect = (fade_to_color_effect_t*)effect;
  unsigned long elapsed_time = timer_time_since(fade_to_color_effect->stop_time);
  fade_to_color_effect->start_time += elapsed_time;
}


void fade_to_color_effect_step(effect_t* effect)
{
  fade_to_color_effect_t* fade_to_color_effect = (fade_to_color_effect_t*)effect;
  unsigned long elapsed_time = timer_time_since(fade_to_color_effect->start_time);
  unsigned long step_long = (unsigned long) (fade_to_color_effect->step * elapsed_time);
  unsigned char step = step_long > UCHAR_MAX ? UCHAR_MAX : (unsigned char) step_long;
  const palette_t* original_palette = fade_to_color_effect->palette;
  palette_t new_palette = {0};
  int modified = 0;
  int i = fade_to_color_effect->start_color;

  for(; i < fade_to_color_effect->end_color; ++i)
  {
    const int red_difference = fade_to_color_effect->target.red - original_palette->colors[i].red;
    const int green_difference = fade_to_color_effect->target.green - original_palette->colors[i].green;
    const int blue_difference = fade_to_color_effect->target.blue - original_palette->colors[i].blue;
    new_palette.colors[i] = fade_to_color_effect->target;

    if(abs(red_difference) > step)
    {
      ++modified;
      new_palette.colors[i].red = original_palette->colors[i].red + ((red_difference > 0) ? step : -step);
    }
    if(abs(green_difference) > step)
    {
      ++modified;
      new_palette.colors[i].green = original_palette->colors[i].green + ((green_difference > 0) ? step : -step);
    }    
    if(abs(blue_difference) > step)
    {
      ++modified;
      new_palette.colors[i].blue = original_palette->colors[i].blue + ((blue_difference > 0) ? step : -step);
    }    
  }
  render_set_palette(&new_palette);
  fade_to_color_effect->finished = (elapsed_time >= fade_to_color_effect->time);
}

bool fade_to_color_effect_finished(effect_t* effect)
{
  const fade_to_color_effect_t* fade_to_color_effect = (const fade_to_color_effect_t*)effect;
  return fade_to_color_effect->finished;
}

void fade_to_color_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_fade_to_color_effect(
    const palette_t* palette,
    const palette_entry_t* target,
    int start_color,
    int end_color,
    unsigned long time)
{
  fade_to_color_effect_t* effect = (fade_to_color_effect_t*) malloc(sizeof(fade_to_color_effect_t));
  if(effect)
  {
    effect->palette = palette;
    effect->target = *target;
    effect->start_time = 0;
    effect->start_color = start_color;
    effect->end_color = end_color;
    effect->time = time;
    effect->finished = false;

    effect->effect.finished = fade_to_color_effect_finished;
    effect->effect.free = fade_to_color_effect_free;
    effect->effect.step = fade_to_color_effect_step;
    effect->effect.start = fade_to_color_effect_start;
    effect->effect.stop = fade_to_color_effect_stop;
    effect->effect.resume = fade_to_color_effect_resume;
  } 
  return &(effect->effect);
}
