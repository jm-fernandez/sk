#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "keyboard.h"
#include "effect.h"
#include "state_story.h"
#include "keyconfig.h"
#include "state_ctrl.h"
#include "sprite.h"
#include "render.h"

#include "set_background_effect.h"
#include "set_palette_effect.h"
#include "util.h"
#include "fade_to_color_effect.h"
#include "wait_effect.h"
#include "serialize_effect.h"
#include "show_image_effect.h"
#include "log.h"

#define STORY_IMAGE_HAWAI_TEXT    0
#define STORY_IMAGE_HAWAI         1
#define STORY_IMAGE_CALL          2
#define STORY_IMAGE_LATE          3
#define STORY_IMAGE_GIJON_TEXT    4
#define STORY_IMAGE_HIGH_SCHOOL   5
#define STORY_IMAGE_LAST_TEXT     6
#define STORY_IMAGE_FAIL_TEXT     7
#define STORY_IMAGE_DISC_1        8
#define STORY_IMAGE_DISC_2        9
#define STORY_IMAGE_DISC_3        10
#define STORY_IMAGE_COUNT         11

static const char* story_image_file_names[] = {
  "hawai",
  "beach",
  "call",
  "tarde",
  "gijon",
  "insti",
  "ultimo",
  "mn",
  "m",
  "p",
  "mu",
};

#define EFFECT_COUNT                        (5 * STORY_IMAGE_COUNT)

#define FADEOUT_TIME                        2500000
#define WAIT_TIME                           1000000
#define WAIT_TIME_LONG                      3000000

typedef struct story_state_t_
{
  state_t state;
  int pause_count;
  int exit_count;
  sprite_t* black_screen;
  sprite_t* sprites[STORY_IMAGE_COUNT];
  effect_t* set_palette_effects[STORY_IMAGE_COUNT];
  effect_t* fade_out_effects[STORY_IMAGE_COUNT];
  effect_t* show_image_effects[STORY_IMAGE_COUNT];
  effect_t* set_background_effect;
  effect_t* wait_effect;
  effect_t* wait_long_effect;
  effect_t* steps[EFFECT_COUNT];
} story_state_t;

static void story_start(struct state_t_* state)
{
  story_state_t* story_state = (story_state_t*)state;
  if(story_state)
  {
    keyboard_get_key_status(KEY_CONFIG_PAUSE, &story_state->pause_count);
    keyboard_get_key_status(KEY_CONFIG_EXIT, &story_state->exit_count);
  	effect_start(story_state->steps[EFFECT_COUNT - 1]);
    render_show(false);
  }
}

static void story_stop(struct state_t_* state)
{
  story_state_t* story_state = (story_state_t*)state;
  effect_stop(story_state->steps[EFFECT_COUNT - 1]);
}

static void story_resume(struct state_t_* state)
{
  story_state_t* story_state = (story_state_t*)state;
  keyboard_get_key_status(KEY_CONFIG_PAUSE, &story_state->pause_count);
  keyboard_get_key_status(KEY_CONFIG_EXIT, &story_state->exit_count);
  effect_resume(story_state->steps[EFFECT_COUNT - 1]);
}

static bool story_step(struct state_t_* state)
{
  story_state_t* story_state = (story_state_t*)state;
  if(story_state)
  {
    int current_pause_count = 0;
    int current_exit_count = 0;
    const bool pause = keyboard_get_key_status(KEY_CONFIG_PAUSE, &current_pause_count);
    const bool exit = keyboard_get_key_status(KEY_CONFIG_EXIT, &current_exit_count);

    if(pause && current_pause_count != story_state->pause_count)
    {
      state_ctrl_set(game_state_pause);
    }
    else if(exit && current_exit_count != story_state->exit_count)
    {
      state_ctrl_set(game_state_fight);
    }
    else
    {
      bool finished = effect_finished(story_state->steps[EFFECT_COUNT - 1]);
      if(!finished)
      {
        effect_step(story_state->steps[EFFECT_COUNT - 1]);
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

static void story_free(struct state_t_* state)
{
  story_state_t* story_state = (story_state_t*)state;
  if(story_state)
  {
    int i = 0;

    if(story_state->wait_effect)
    {
      effect_free(story_state->wait_effect);
    }

    if(story_state->set_background_effect)
    {
      effect_free(story_state->set_background_effect);
    }

    for(;i < EFFECT_COUNT; ++i)
    {
      if(story_state->steps[i] != NULL)
      {
        effect_free(story_state->steps[i]);
        story_state->steps[i] = NULL;
      }
    }

    for(;i < STORY_IMAGE_COUNT; ++i)
    {
      sprite_free(story_state->sprites[i]);
      effect_free(story_state->fade_out_effects[i]);
      effect_free(story_state->set_palette_effects[i]);
      effect_free(story_state->show_image_effects[i]);
    }

    if(story_state->black_screen)
    {
        sprite_free(story_state->black_screen);
        story_state->black_screen = NULL;
    }

    free(state);
    state = NULL;
  }
}

state_t* state_story_create()
{
  story_state_t* result = (story_state_t*) malloc(sizeof(story_state_t));
  if(result)
  {
    int i = 0;
    int j = 0;
    char file_path[256];
    const palette_entry_t black_palette_entry = {0, 0, 0, 0};
    const square_t zoom_start = {319, 239, 2, 2};
    const square_t zoom_stop = {320 - 120, 240 - 120, 240, 240};
    const square_t zoom_stop_big = {320 - 200, 240 - 200, 400, 400};
    char error[256];

    memset(result, 0, sizeof(*result));

    result->state.free = story_free;
    result->state.resume = story_resume;
    result->state.start = story_start;
    result->state.step = story_step;
    result->state.stop = story_stop;

    result->black_screen = create_solid_sprite(640, 480, black_palette_entry);
    result->set_background_effect = create_set_background_effect(result->black_screen);
    result->wait_effect = create_wait_effect(WAIT_TIME);
    result->wait_long_effect = create_wait_effect(WAIT_TIME_LONG);

    for(; i < STORY_IMAGE_COUNT; ++i)
    {
      square_t square = {0};
      int sx = 0;
      int sy = 0;

      sprintf(file_path,"assets\\story\\%s.bmp", story_image_file_names[i]);
      result->sprites[i] = create_bmp_sprite(file_path);
      if(!result->sprites[i])
      {
        sprintf(error, "error opening %s", story_image_file_names[i]);
        log_record(error);
      }

      sx = sprite_width(result->sprites[i]);
      sy = sprite_height(result->sprites[i]);
      
      square.x = 320 - sx / 2;
      square.y = 240 - sy / 2;
      square.sx = sx;
      square.sy = sy;

      result->set_palette_effects[i] = create_set_palette_effect(sprite_palette(result->sprites[i] ), true);
      result->fade_out_effects[i] = create_fadeout_effect(sprite_palette(result->sprites[i]), FADEOUT_TIME);
      result->show_image_effects[i] = create_show_image_effect(result->sprites[i], &square, true);

      if(j == 0)
      {
        result->steps[j] = create_serialize_effect(
          result->set_background_effect,
          result->set_palette_effects[i]);
      }
      else
      {
        ++j;
        result->steps[j] = create_serialize_effect(
          result->steps[j-1],
          result->set_palette_effects[i]);      
      }
      
      ++j;

      result->steps[j] = create_serialize_effect(
        result->steps[j-1],
        result->show_image_effects[i]);

      if(i >= STORY_IMAGE_DISC_1)
      {
        ++j;
        result->steps[j] = create_serialize_effect(
          result->steps[j-1],
          result->wait_long_effect);
      }
      else
      {
        ++j;
        result->steps[j] = create_serialize_effect(
          result->steps[j-1],
          result->wait_effect);
        
      }

      ++j;
      result->steps[j] = create_serialize_effect(
        result->steps[j-1],
        result->fade_out_effects[i]);

      ++j;
      result->steps[j] = create_serialize_effect(
        result->steps[j-1],
        result->set_background_effect);
      }
  }
  return result == NULL ? NULL : &(result->state);
}
