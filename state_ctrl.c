#include <string.h>

#include "state_ctrl.h"
#include "state.h"
#include "state_fight.h"
#include "state_intro.h"
#include "state_pause.h"
#include "state_story.h"

typedef struct state_ctrl_t_
{
  state_t* states[game_state_count];
  bool started[game_state_count];
}state_ctrl_t;

static state_ctrl_t state_ctrl;

bool state_ctrl_initialize()
{
    memset(&state_ctrl, 0, sizeof(state_ctrl));
    state_ctrl.states[game_state_intro] = state_intro_create();
    state_ctrl.states[game_state_pause] = state_pause_create();
    state_ctrl.states[game_state_fight] = state_fight_create();
    state_ctrl.states[game_state_story] = state_story_create();
    return (state_ctrl.states[game_state_intro] != NULL &&
     state_ctrl.states[game_state_pause] != NULL        &&
     state_ctrl.states[game_state_fight] != NULL);
}

void state_ctrl_deinitialize()
{
  int i = 0;
  
  state_ctrl.states[game_state_current] = NULL;

  for(; i < game_state_count; ++i)
  {
    if(state_ctrl.states[i])
    {
      state_free(state_ctrl.states[i]);
      state_ctrl.states[i] = NULL;
    }
  }
}

bool state_ctrl_step()
{
  bool result = false;
  if(!state_ctrl.states[game_state_current])
  {
    state_ctrl.states[game_state_current] = state_ctrl.states[game_state_intro];
    state_start(state_ctrl.states[game_state_current]);
    result = true;
  }
  else
  {
    result = state_step(state_ctrl.states[game_state_current]);
  }
  return result;
}

void state_ctrl_set(game_state_t state)
{
  state_t* current_state = state_ctrl.states[game_state_current];
  state_t* new_state = state_ctrl.states[state];
  if(new_state != current_state)
  {
    if(current_state != NULL)
    {
      state_stop(current_state);
    }
    if(new_state != NULL)
    {
      if(state_ctrl.started[state])
      {
        state_resume(new_state);
      }
      else
      {
        state_start(new_state);
        state_ctrl.started[state] = true;
      }
    }
    state_ctrl.states[game_state_current] = new_state;
  }
}

state_t* state_ctrl_get_current()
{
  return state_ctrl.states[game_state_current];
}

void state_ctrl_resume_by_pointer(state_t* state)
{
  state_t* current_state = state_ctrl.states[game_state_current];
  if(current_state != state)
  {
      state_stop(current_state);
      state_resume(state);
      state_ctrl.states[game_state_current] = state;
  }
}
