#include <stdbool.h>
#include <stdlib.h>

#include "wait_effect.h"
#include "timer.h"

typedef struct wait_effect_t_ {
  effect_t effect;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long time;
  bool finished;
} wait_effect_t;

void wait_effect_start(effect_t* effect)
{
  wait_effect_t* wait_effect = (wait_effect_t*)effect;
  wait_effect->finished = false;
  timer_read(&wait_effect->start_time);
}

void wait_effect_stop(effect_t* effect)
{
  wait_effect_t* wait_effect = (wait_effect_t*)effect;
  timer_read(&wait_effect->stop_time); 
}

void wait_effect_resume(effect_t* effect)
{
  wait_effect_t* wait_effect = (wait_effect_t*)effect;
  const unsigned long elapsed = timer_time_since(wait_effect->stop_time);
  wait_effect->start_time += elapsed;  
}

void wait_effect_step(effect_t* effect)
{
  wait_effect_t* wait_effect = (wait_effect_t*)effect;
  const unsigned long elapsed = timer_time_since(wait_effect->start_time);
  wait_effect->finished = (elapsed > wait_effect->time);
}

bool wait_effect_finished(effect_t* effect)
{
  const wait_effect_t* wait_effect = (const wait_effect_t*)effect;
  return wait_effect->finished;
}

void wait_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_wait_effect(unsigned long time)
{
  wait_effect_t* effect = (wait_effect_t*) malloc(sizeof(wait_effect_t));
  if(effect)
  {
    effect->start_time = 0U;;
    effect->time = time;
    effect->finished = false;

    effect->effect.finished = wait_effect_finished;
    effect->effect.free = wait_effect_free;
    effect->effect.step = wait_effect_step;
    effect->effect.start = wait_effect_start;
    effect->effect.stop = wait_effect_stop;
    effect->effect.resume = wait_effect_resume;
  } 
  return &(effect->effect);
}
