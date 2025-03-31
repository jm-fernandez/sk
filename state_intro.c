#include <stdlib.h>
#include <string.h>

#include "keyboard.h"
#include "effect.h"
#include "state_intro.h"
#include "keyconfig.h"
#include "state_ctrl.h"
#include "sprite.h"
#include "render.h"

#include "set_background_effect.h"
#include "set_palette_effect.h"
#include "util.h"
#include "zoom_effect.h"
#include "fade_to_color_effect.h"
#include "wait_effect.h"
#include "serialize_effect.h"

#define EFFECT_SET_BACKGROUND               0
#define EFFECT_SET_DOS_LOGO_PALETTE         1
#define EFFECT_SET_CONTEXT_LOGO_PALETTE     2
#define EFFECT_SET_SK_LOGO_PALETTE          3
#define EFFECT_DOS_LOGO_ZOOM                4
#define EFFECT_DOS_CONTEXT_ZOOM             5
#define EFFECT_SK_LOGO_ZOOM                 6
#define EFFECT_DOS_LOGO_FADEOUT             7
#define EFFECT_DOS_CONTEXT_FADEOUT          8
#define EFFECT_SK_LOGO_FADEOUT              9
#define EFFECT_WAIT                         10
#define EFFECT_WAIT_LONG                    11

#define EFFECT_STEP_1                       12
#define EFFECT_STEP_2                       13
#define EFFECT_STEP_3                       14
#define EFFECT_STEP_4                       15
#define EFFECT_STEP_5                       16
#define EFFECT_STEP_6                       17
#define EFFECT_STEP_7                       18
#define EFFECT_STEP_8                       19
#define EFFECT_STEP_9                       20
#define EFFECT_STEP_10                      21
#define EFFECT_STEP_11                      22
#define EFFECT_STEP_12                      23
#define EFFECT_STEP_13                      24
#define EFFECT_INTRO                        25

#define EFFECT_COUNT                        26

#define ZOOM_TIME                           2500000
#define FADEOUT_TIME                        2500000
#define WAIT_TIME                           1000000
#define WAIT_TIME_LONG                      3000000

typedef struct intro_State_t_
{
  state_t state;
  sprite_t* black_screen;
  sprite_t* dos_logo;
  sprite_t* dos_context_logo;
  sprite_t* game_logo;
  effect_t* effects[EFFECT_COUNT];
} intro_state_t;

void intro_start(struct state_t_* state)
{
  intro_state_t* intro_state = (intro_state_t*)state;
  if(intro_state)
  {
  	effect_start(intro_state->effects[EFFECT_INTRO]);
    render_show(false);
  }
}

void intro_stop(struct state_t_* state)
{
  intro_state_t* intro_state = (intro_state_t*)state;
  effect_stop(intro_state->effects[EFFECT_INTRO]);
}

void intro_resume(struct state_t_* state)
{
  intro_state_t* intro_state = (intro_state_t*)state;
  effect_resume(intro_state->effects[EFFECT_INTRO]);
}

bool intro_step(struct state_t_* state)
{
  intro_state_t* intro_state = (intro_state_t*)state;
  if(intro_state)
  {
    const int key = keyboard_get_key();
    if(key == KEY_CONFIG_PAUSE)
    {
      state_ctrl_set(game_state_pause);
    }
    else if(key == KEY_CONFIG_EXIT)
    {
      state_ctrl_set(game_state_story);
    }
    else
    {
      bool finished = effect_finished(intro_state->effects[EFFECT_INTRO]);
      if(!finished)
      {
        effect_step(intro_state->effects[EFFECT_INTRO]);
        render_show(false);
      }
      else
      {
        state_ctrl_set(game_state_fight);
      }
    }
  }
  return true;
}

void intro_free(struct state_t_* state)
{
  intro_state_t* intro_state = (intro_state_t*)state;
  if(intro_state)
  {
    int i = 0;

    for(;i < EFFECT_COUNT; ++i)
    {
      if(intro_state->effects[i] != NULL)
      {
        effect_free(intro_state->effects[i]);
        intro_state->effects[i] = NULL;
      }
    }

    if(intro_state->dos_logo)
    {
        sprite_free(intro_state->dos_logo);
        intro_state->dos_logo = NULL;
    }

    if(intro_state->dos_context_logo)
    {
      sprite_free(intro_state->dos_context_logo);
      intro_state->dos_context_logo = NULL;
    }

    if(intro_state->game_logo)
    {
      sprite_free(intro_state->game_logo);
      intro_state->game_logo = NULL;
    }

    if(intro_state->black_screen)
    {
        sprite_free(intro_state->black_screen);
        intro_state->black_screen = NULL;
    }


    free(state);
    state = NULL;
  }
}

state_t* state_intro_create()
{
  intro_state_t* result = (intro_state_t*) malloc(sizeof(intro_state_t));
  if(result)
  {
    const palette_entry_t black_palette_entry = {0, 0, 0, 0};

    memset(result, 0, sizeof(*result));

    result->state.free = intro_free;
    result->state.resume = intro_resume;
    result->state.start = intro_start;
    result->state.step = intro_step;
    result->state.stop = intro_stop;

    result->black_screen = create_solid_sprite(640, 480, black_palette_entry);
    result->dos_logo = create_bmp_sprite("assets\\intro\\msdos.bmp");
    result->dos_context_logo = create_bmp_sprite("assets\\intro\\logoc.bmp");
    result->game_logo = create_bmp_sprite("assets\\intro\\sk_logo.bmp");
    
    if(result->black_screen && result->dos_logo && result->dos_context_logo && result->game_logo)
    {
      int i = 0;
      const square_t zoom_start = {319, 239, 2, 2};
      const square_t zoom_stop = {320 - 120, 240 - 120, 240, 240};
      const square_t zoom_stop_big = {320 - 200, 240 - 200, 400, 400};

      result->effects[EFFECT_SET_BACKGROUND] = create_set_background_effect(result->black_screen);
      result->effects[EFFECT_SET_DOS_LOGO_PALETTE] = create_set_palette_effect(sprite_palette(result->dos_logo), true);
      result->effects[EFFECT_SET_CONTEXT_LOGO_PALETTE] = create_set_palette_effect(sprite_palette(result->dos_context_logo), true);
      result->effects[EFFECT_SET_SK_LOGO_PALETTE] = create_set_palette_effect(sprite_palette(result->game_logo), true);

      result->effects[EFFECT_DOS_LOGO_ZOOM] = create_zoom_effect(result->dos_logo, &zoom_start, &zoom_stop, ZOOM_TIME, true);
      result->effects[EFFECT_DOS_CONTEXT_ZOOM] = create_zoom_effect(result->dos_context_logo, &zoom_start, &zoom_stop, ZOOM_TIME, true);
      result->effects[EFFECT_SK_LOGO_ZOOM] = create_zoom_effect(result->game_logo, &zoom_start, &zoom_stop_big, ZOOM_TIME, true);

      result->effects[EFFECT_DOS_LOGO_FADEOUT] = create_fadeout_effect(sprite_palette(result->dos_logo), FADEOUT_TIME);
      result->effects[EFFECT_DOS_CONTEXT_FADEOUT] = create_fadeout_effect(sprite_palette(result->dos_context_logo), FADEOUT_TIME);
      result->effects[EFFECT_SK_LOGO_FADEOUT] = create_fadeout_effect(sprite_palette(result->game_logo), FADEOUT_TIME);

      result->effects[EFFECT_WAIT] = create_wait_effect(WAIT_TIME);
      result->effects[EFFECT_WAIT_LONG] = create_wait_effect(WAIT_TIME_LONG);


      result->effects[EFFECT_STEP_1] = create_serialize_effect(
          result->effects[EFFECT_SET_BACKGROUND],
          result->effects[EFFECT_SET_DOS_LOGO_PALETTE]);
      
      result->effects[EFFECT_STEP_2] = create_serialize_effect(
        result->effects[EFFECT_STEP_1],
        result->effects[EFFECT_DOS_LOGO_ZOOM]);
      
      result->effects[EFFECT_STEP_3] = create_serialize_effect(
        result->effects[EFFECT_STEP_2], 
        result->effects[EFFECT_WAIT]);

      result->effects[EFFECT_STEP_4] = create_serialize_effect(
        result->effects[EFFECT_STEP_3],
        result->effects[EFFECT_DOS_LOGO_FADEOUT]);
   
      result->effects[EFFECT_STEP_5] = create_serialize_effect(
        result->effects[EFFECT_STEP_4],
        result->effects[EFFECT_SET_BACKGROUND]);

      result->effects[EFFECT_STEP_6] = create_serialize_effect(
        result->effects[EFFECT_STEP_5],
        result->effects[EFFECT_SET_CONTEXT_LOGO_PALETTE]);
  
      result->effects[EFFECT_STEP_7] = create_serialize_effect(
          result->effects[EFFECT_STEP_6],
          result->effects[EFFECT_DOS_CONTEXT_ZOOM]);
  
      result->effects[EFFECT_STEP_8] = create_serialize_effect(
          result->effects[EFFECT_STEP_7],
          result->effects[EFFECT_WAIT]);
  
      result->effects[EFFECT_STEP_9] = create_serialize_effect(
          result->effects[EFFECT_STEP_8],
          result->effects[EFFECT_DOS_CONTEXT_FADEOUT]);

      result->effects[EFFECT_STEP_10] = create_serialize_effect(
            result->effects[EFFECT_STEP_9],
            result->effects[EFFECT_SET_BACKGROUND]);

      result->effects[EFFECT_STEP_10] = create_serialize_effect(
        result->effects[EFFECT_STEP_9],
        result->effects[EFFECT_SET_SK_LOGO_PALETTE]);

      result->effects[EFFECT_STEP_11] = create_serialize_effect(
          result->effects[EFFECT_STEP_10],
          result->effects[EFFECT_SK_LOGO_ZOOM]);

      result->effects[EFFECT_STEP_12] = create_serialize_effect(
          result->effects[EFFECT_STEP_11],
          result->effects[EFFECT_WAIT_LONG]);

      result->effects[EFFECT_STEP_13] = create_serialize_effect(
            result->effects[EFFECT_STEP_12],
            result->effects[EFFECT_SK_LOGO_FADEOUT]);
  
      result->effects[EFFECT_INTRO] = create_serialize_effect(
           result->effects[EFFECT_STEP_13],
           result->effects[EFFECT_SET_BACKGROUND]);

      for(; i < EFFECT_COUNT; ++i)
      {
        if(result->effects[i] == NULL)
        {
          intro_free(&(result->state));
          result = NULL;
          break;
        }
      }
    }
  }
  return result == NULL ? NULL : &(result->state);
}
