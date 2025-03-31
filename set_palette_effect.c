#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "set_palette_effect.h"
#include "render.h"

typedef struct set_palette_effect_t_ {
  effect_t effect;
  bool clean;
  const palette_t* palette;
}set_palette_effect_t;

static void set_palette_effect_start(effect_t* effect)
{
  set_palette_effect_t* set_pallete_effect = (set_palette_effect_t*) effect;
  if(set_pallete_effect->clean)
  {
    render_clean();
  }
  render_set_palette(set_pallete_effect->palette);
}

static void set_palette_effect_stop(effect_t* effect)
{
}

static void set_palette_effect_resume(effect_t* effect)
{
}

static void set_palette_effect_step(effect_t* effect)
{
}

static bool set_palette_effect_finished(effect_t* effect)
{
  return true;
}

static void set_palette_effect_free(effect_t* effect)
{
  free(effect);
  effect = NULL;
}

effect_t* create_set_palette_effect(const palette_t* palette, bool clean)
{
  set_palette_effect_t* effect = (set_palette_effect_t*) malloc(sizeof(set_palette_effect_t));
  if(effect)
  {
    effect->palette = palette;
    effect->clean = clean;

    effect->effect.finished = set_palette_effect_finished;
    effect->effect.free = set_palette_effect_free;
    effect->effect.step = set_palette_effect_step;
    effect->effect.start = set_palette_effect_start;
    effect->effect.stop = set_palette_effect_stop;
    effect->effect.resume = set_palette_effect_resume;
  } 
  return &(effect->effect);
}
