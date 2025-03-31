#ifndef STATE_H__
#define STATE_H__

#include <stdbool.h>

typedef struct state_t_ 
{
  void(*start)(struct state_t_* state);
  void(*stop)(struct state_t_* state);
  void(*resume)(struct state_t_* state);
  bool (*step)(struct state_t_* state);
  void (*free)(struct state_t_* state);
} state_t;

static inline void state_start(state_t* state)
{
  state->start(state);
}

static inline void state_stop(state_t* state)
{
  state->stop(state);
}

static inline void state_resume(state_t* state)
{
  state->resume(state);
}

static inline bool state_step(state_t* state)
{
  return state->step(state);
}

static inline void state_free(state_t* state)
{
  state->free(state);
}

#endif // STATE_H__
