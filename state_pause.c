#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "keyboard.h"
#include "keyconfig.h"
#include "state.h"
#include "state_ctrl.h"
#include "state.h"
#include "render.h"

typedef struct pause_state_t_
{
  state_t state;
  int pause_count;
  int exit_count;
  state_t* previous_state;
  palette_t previous_palette;
  palette_t new_palette;
} pause_state_t;

static void pause_start(state_t* state)
{
  pause_state_t* pause_state = (pause_state_t*)state;
  if(pause_state)
  {
    int i = 0;

    keyboard_get_key_status(KEY_CONFIG_PAUSE, &pause_state->pause_count);
    keyboard_get_key_status(KEY_CONFIG_EXIT, &pause_state->exit_count);

    pause_state->previous_state = state_ctrl_get_current();
    pause_state->previous_palette = *render_get_palette();

    for(; i < 256; ++i)
    {
      const int red = pause_state->previous_palette.colors[i].red;
      const int green = pause_state->previous_palette.colors[i].green;
      const int blue = pause_state->previous_palette.colors[i].blue;
      const int media = (red + green + blue) / 3;
      pause_state->new_palette.colors[i].red = media;
      pause_state->new_palette.colors[i].green = media;
      pause_state->new_palette.colors[i].blue = media;
    }

    render_set_palette(&(pause_state->new_palette));
    render_show(false);
  }
}

static void pause_stop(state_t* state)
{
  pause_state_t* pause_state = (pause_state_t*)state;
  if(pause_state)
  {
    pause_state->previous_state = NULL;
    render_set_palette(&(pause_state->previous_palette));
    render_show(false);
  }
}

static void pause_resume(state_t* state)
{
  pause_start(state);
}

static bool pause_step(state_t* state)
{
  bool result = true;

  int current_pause_count = 0;
  int current_exit_count = 0;
  pause_state_t* pause_state = (pause_state_t*)state;
  const bool pause = keyboard_get_key_status(KEY_CONFIG_PAUSE, &current_pause_count);
  const bool exit = keyboard_get_key_status(KEY_CONFIG_EXIT, &current_exit_count);
  if(pause && current_pause_count != pause_state->pause_count)
  {
    pause_state_t* pause_state = (pause_state_t*)state;
    state_ctrl_resume_by_pointer(pause_state->previous_state);
  }
  else if(exit && current_exit_count != pause_state->exit_count)
  {
    result = false;
  }

  return result;
}

void pause_free(state_t* state)
{
  pause_state_t* pause_state = (pause_state_t*)state;
  if(pause_state)
  {
    free(pause_state);
  }
}

state_t* state_pause_create()
{
  pause_state_t* result = (pause_state_t*) malloc(sizeof(pause_state_t));
  if(result)
  {
    memset(result, 0, sizeof(*result));

    result->state.free = pause_free;
    result->state.resume = pause_resume;
    result->state.start = pause_start;
    result->state.step = pause_step;
    result->state.stop = pause_stop;
  }
  return result == NULL ? NULL : &(result->state);
}
