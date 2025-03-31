#include <stdbool.h>
#include <stdlib.h>

#include "effect.h"

typedef struct serialize_effect_t_ {
  effect_t effect;
  effect_t* first_effect;
  effect_t* second_effect;
  bool first_effect_finished;
}serialize_effect_t;

void serialize_effect_start(effect_t* effect)
{
  serialize_effect_t* serialize_effect = (serialize_effect_t*)effect;
  effect_start(serialize_effect->first_effect);
}

void serialize_effect_stop(effect_t* effect)
{
  serialize_effect_t* serialize_effect = (serialize_effect_t*)effect;
  if(serialize_effect->first_effect_finished)
  {
    effect_stop(serialize_effect->second_effect);
  }
  else
  {
    effect_stop(serialize_effect->first_effect);  
  }
}

void serialize_effect_resume(effect_t* effect)
{
  serialize_effect_t* serialize_effect = (serialize_effect_t*)effect;
  if(serialize_effect->first_effect_finished)
  {
    effect_resume(serialize_effect->second_effect);
  }
  else
  {
    effect_resume(serialize_effect->first_effect);  
  }
}

void serialize_effect_step(effect_t* effect)
{
  serialize_effect_t* serialize_effect = (serialize_effect_t*)effect;
  if(serialize_effect->first_effect_finished)
  {
    effect_step(serialize_effect->second_effect);
  }
  else if(effect_finished(serialize_effect->first_effect))
  {
    effect_start(serialize_effect->second_effect);
    serialize_effect->first_effect_finished = true;
  }
  else
  {
    effect_step(serialize_effect->first_effect);
  }
}

bool serialize_effect_finished(effect_t* effect)
{
  const serialize_effect_t* serialize_effect = (const serialize_effect_t*) effect;
  return serialize_effect->first_effect_finished && effect_finished(serialize_effect->second_effect);
}

void serialize_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_serialize_effect(effect_t* first_effect, effect_t* second_effect)
{
  serialize_effect_t* effect = (serialize_effect_t*) malloc(sizeof(serialize_effect_t));
  if(effect)
  {
    effect->first_effect = first_effect;
    effect->second_effect = second_effect;
    effect->first_effect_finished = false;

    effect->effect.finished = serialize_effect_finished;
    effect->effect.free = serialize_effect_free;
    effect->effect.step = serialize_effect_step;
    effect->effect.start = serialize_effect_start;
    effect->effect.stop = serialize_effect_stop;
    effect->effect.resume = serialize_effect_resume;
  } 
  return &(effect->effect);
}
