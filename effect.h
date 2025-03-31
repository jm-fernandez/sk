#ifndef EFFECT_H__
#define EFFECT_H__

#include <stdbool.h>

typedef struct effect_t_ 
{
  void(*start)(struct effect_t_* effect);
  void(*stop)(struct effect_t_* effect);
  void (*resume)(struct effect_t_* effect);
  void (*step)(struct effect_t_* effect);
  bool(*finished)(struct effect_t_* animation);
  void (*free)(struct effect_t_* animation);
} effect_t;

static inline void effect_start(effect_t* effect)
{
  effect->start(effect);
}

static inline void effect_stop(effect_t* effect)
{
  effect->stop(effect);
}

static inline void effect_resume(effect_t* effect)
{
  effect->resume(effect);
}

static inline void effect_step(effect_t* effect)
{
  effect->step(effect);
}

static inline bool effect_finished(effect_t* effect)
{
  return effect->finished(effect);
}

static inline void effect_free(effect_t* effect)
{
  effect->free(effect);
}

#endif // EFFECT_H__
