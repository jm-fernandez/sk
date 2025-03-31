#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "set_background_effect.h"
#include "render.h"

typedef struct set_background_effect_t_ {
  effect_t effect;
  const sprite_t* sprite;
}set_background_effect_t;


static void set_background_effect_start(effect_t* effect)
{
  set_background_effect_t* set_background_effect = (set_background_effect_t*)effect;
  render_set_background(set_background_effect->sprite);
}

static void set_background_effect_stop(effect_t* effect)
{
}

static void set_background_effect_resume(effect_t* effect)
{
}

static void set_background_effect_step(effect_t* effect)
{ 
}

static bool set_background_effect_finished(effect_t* effect)
{
  return true;
}

static void set_background_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_set_background_effect(const sprite_t* sprite)
{
  set_background_effect_t* effect = (set_background_effect_t*) malloc(sizeof(set_background_effect_t));
  if(effect)
  {
    effect->sprite = sprite;

    effect->effect.finished = set_background_effect_finished;
    effect->effect.free = set_background_effect_free;
    effect->effect.step = set_background_effect_step;
    effect->effect.start = set_background_effect_start;
    effect->effect.stop = set_background_effect_stop;
    effect->effect.resume = set_background_effect_resume;
  }
  return &(effect->effect);
}
