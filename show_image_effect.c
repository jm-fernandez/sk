#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "show_image_effect.h"
#include "render.h"

typedef struct show_image_effect_t_ {
  effect_t effect;
  bool clean;
  const sprite_t* sprite;
  square_t area;
}show_image_effect_t;


static void show_image_effect_start(effect_t* effect)
{
  show_image_effect_t* show_image_effect = (show_image_effect_t*)effect;
  if(show_image_effect->clean)
  {
    render_clean();
  }
  render_sprite(show_image_effect->sprite, &show_image_effect->area, false);
}

static void show_image_effect_stop(effect_t* effect)
{ 
}

static void show_image_effect_resume(effect_t* effect)
{
}

static void show_image_effect_step(effect_t* effect)
{ 
}

static bool show_image_effect_finished(effect_t* effect)
{
  return true;
}

static void show_image_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_show_image_effect(const sprite_t* sprite, const square_t* area, bool clean)
{
  show_image_effect_t* effect = (show_image_effect_t*) malloc(sizeof(show_image_effect_t));
  if(effect)
  {
    effect->clean = clean;
    effect->sprite = sprite;
    effect->area = *area;

    effect->effect.finished = show_image_effect_finished;
    effect->effect.free = show_image_effect_free;
    effect->effect.step = show_image_effect_step;
    effect->effect.start = show_image_effect_start;
    effect->effect.stop = show_image_effect_stop;
    effect->effect.resume = show_image_effect_resume;
  } 
  return &(effect->effect);
}
