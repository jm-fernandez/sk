#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "zoom_effect.h"
#include "render.h"
#include "timer.h"

typedef struct zoom_effect_t_ {
  effect_t effect;
  bool clean;
  const sprite_t* sprite;
  square_t start;
  square_t stop;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long time;
  bool finished;
}zoom_effect_t;


void zoom_effect_start(effect_t* effect)
{
  zoom_effect_t* zoom_effect = (zoom_effect_t*)effect;
  square_t square = {0};

  square.x = zoom_effect->start.x;
  square.y = zoom_effect->start.y;
  square.sx = zoom_effect->start.sx;
  square.sy = zoom_effect->start.sy;

  if(zoom_effect->clean)
  {
    render_clean();
  }

  render_sprite(zoom_effect->sprite, &square, false);  
  timer_read(&zoom_effect->start_time);
}

static void zoom_effect_stop(effect_t* effect)
{
  zoom_effect_t* zoom_effect = (zoom_effect_t*)effect;
  timer_read(&zoom_effect->stop_time);
}

static void zoom_effect_resume(effect_t* effect)
{
  zoom_effect_t* zoom_effect = (zoom_effect_t*)effect;
  const unsigned long elapsed_time = timer_time_since(zoom_effect->stop_time);
  zoom_effect->start_time += elapsed_time;
}

void zoom_effect_step(effect_t* effect)
{
  zoom_effect_t* zoom_effect = (zoom_effect_t*)effect;
  unsigned long elapsed_time = timer_time_since(zoom_effect->start_time);
  double relative_elapsed = (double) elapsed_time / zoom_effect->time;
  square_t square = {0};

  if(relative_elapsed >= 1.0)
  {
    square.x = zoom_effect->stop.x;
    square.y = zoom_effect->stop.y;
    square.sx = zoom_effect->stop.sx;
    square.sy = zoom_effect->stop.sy;
    zoom_effect->finished = true;
  }
  else
  {
    square.x = zoom_effect->start.x + (int)((zoom_effect->stop.x - zoom_effect->start.x) * relative_elapsed);
    square.y = zoom_effect->start.y + (int)((zoom_effect->stop.y - zoom_effect->start.y) * relative_elapsed);
    square.sx = zoom_effect->start.sx + (int)((zoom_effect->stop.sx - zoom_effect->start.sx) * relative_elapsed);
    square.sy = zoom_effect->start.sy + (int)((zoom_effect->stop.sy - zoom_effect->start.sy) * relative_elapsed);
  }
  
  if(zoom_effect->clean)
  {
    render_clean();
  }
  render_sprite(zoom_effect->sprite, &square, false);
}

bool zoom_effect_finished(effect_t* effect)
{
  const zoom_effect_t* zoom_effect = (const zoom_effect_t*)effect;
  return zoom_effect->finished;
}

void zoom_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_zoom_effect(const sprite_t* sprite, const square_t* start, const square_t* stop, int time,  bool clean)
{
  zoom_effect_t* effect = (zoom_effect_t*) malloc(sizeof(zoom_effect_t));
  if(effect)
  {
    effect->clean = clean;
    effect->sprite = sprite;
    effect->start = *start;
    effect->stop = *stop;
    effect->start_time = 0;
    effect->time = time;
    effect->finished = false;

    effect->effect.finished = zoom_effect_finished;
    effect->effect.free = zoom_effect_free;
    effect->effect.step = zoom_effect_step;
    effect->effect.start = zoom_effect_start;
    effect->effect.stop = zoom_effect_stop;
    effect->effect.resume = zoom_effect_resume;
  } 
  return &(effect->effect);
}
