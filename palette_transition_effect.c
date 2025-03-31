#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "palette_transition_effect.h"
#include "effect.h"
#include "render.h"
#include "timer.h"

typedef struct palette_transition_effect_t_ {
  effect_t effect;
  const palette_t* src;
  const palette_t* dst;
  int start_color;
  int end_color;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long time;
  double step;
  bool finished;
}palette_transition_effect_t;

void palette_transition_effect_start(effect_t* effect)
{
  palette_transition_effect_t* palette_transition_effect = (palette_transition_effect_t*) effect;

  const palette_t* src_palette = palette_transition_effect->src;
  const palette_t* dst_palette = palette_transition_effect->dst;

  unsigned long biggest_difference = 0;
  int i = palette_transition_effect->start_color;

  for(; i < palette_transition_effect->end_color; ++i)
  {
    const int red_difference = abs(src_palette->colors[i].red - dst_palette->colors[i].red);
    const int green_difference = abs(src_palette->colors[i].green - dst_palette->colors[i].green);
    const int blue_difference = abs(src_palette->colors[i].blue - dst_palette->colors[i].blue);

    biggest_difference = max(biggest_difference, red_difference);
    biggest_difference = max(biggest_difference, green_difference);
    biggest_difference = max(biggest_difference, blue_difference);
  }

  render_set_palette(src_palette);

  palette_transition_effect->step = (double) biggest_difference / palette_transition_effect->time;
  timer_read(&palette_transition_effect->start_time);
}

void palette_transition_effect_stop(effect_t* effect)
{
  palette_transition_effect_t* palette_transition_effect = (palette_transition_effect_t*)effect;
  timer_read(&(palette_transition_effect->stop_time));
}

void palette_transition_effect_resume(effect_t* effect)
{
  palette_transition_effect_t* palette_transition_effect = (palette_transition_effect_t*)effect;
  unsigned long elapsed_time = timer_time_since(palette_transition_effect->stop_time);
  palette_transition_effect->start_time += elapsed_time;
}

void palette_transition_effect_step(effect_t* effect)
{
  palette_transition_effect_t* palette_transition_effect = (palette_transition_effect_t*)effect;
  unsigned long elapsed_time = timer_time_since(palette_transition_effect->start_time);
  unsigned long step_long = (unsigned long) (palette_transition_effect->step * elapsed_time);
  unsigned char step = step_long > UCHAR_MAX ? UCHAR_MAX : (unsigned char) step_long;
  const palette_t* src_palette = palette_transition_effect->src;
  const palette_t* dst_palette = palette_transition_effect->dst;
  palette_t new_palette;

  int modified = 0;
  int i = palette_transition_effect->start_color;

  for(; i < palette_transition_effect->end_color; ++i)
  {
    const int red_difference = dst_palette->colors[i].red - src_palette->colors[i].red;
    const int green_difference = dst_palette->colors[i].green - src_palette->colors[i].green;
    const int blue_difference = dst_palette->colors[i].blue - src_palette->colors[i].blue;
    new_palette.colors[i] = dst_palette->colors[i];

    if(abs(red_difference) > step)
    {
      ++modified;
      new_palette.colors[i].red = src_palette->colors[i].red + ((red_difference > 0) ? step : -step);
    }
    if(abs(green_difference) > step)
    {
      ++modified;
      new_palette.colors[i].green = src_palette->colors[i].green + ((green_difference > 0) ? step : -step);
    }    
    if(abs(blue_difference) > step)
    {
      ++modified;
      new_palette.colors[i].blue = src_palette->colors[i].blue + ((blue_difference > 0) ? step : -step);
    }    
  }
  render_set_palette(&new_palette);
  palette_transition_effect->finished = (modified == 0);
}

bool palette_transition_effect_finished(effect_t* effect)
{
  const palette_transition_effect_t* palette_transition_effect =
    (const palette_transition_effect_t*)effect;

  return palette_transition_effect->finished;
}

void palette_transition_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_palette_transition_effect(
    const palette_t* src,
    const palette_t* dst,
    int start_color,
    int end_color,
    int time)
{
  palette_transition_effect_t* effect =
    (palette_transition_effect_t*) malloc(sizeof(palette_transition_effect_t));

  if(effect)
  {
    effect->src = src;
    effect->dst = dst;
    effect->start_color = start_color;
    effect->end_color = end_color;
    effect->step = .0;
    effect->start_time = 0;
    effect->time = time;
    effect->finished = false;

    effect->effect.finished = palette_transition_effect_finished;
    effect->effect.free = palette_transition_effect_free;
    effect->effect.step = palette_transition_effect_step;
    effect->effect.start = palette_transition_effect_start;
    effect->effect.stop = palette_transition_effect_stop;
    effect->effect.resume = palette_transition_effect_resume;
  } 
  return &(effect->effect);
}
