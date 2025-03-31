#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "keyboard.h"
#include "keyconfig.h"
#include "state.h"
#include "state_ctrl.h"
#include "state.h"
#include "render.h"
#include "sprite.h"
#include "digits.h"
#include "timer.h"

#include "player.h"
#include "ai.h"

#include "player_standing.h"
#include "player_win.h"
#include "util.h"

#include "play_midi.h"
#include "play_wav.h"

#include "log.h"

#define EXTRA_COLORS_BLACK      0
#define EXTRA_COLORS_GREY       1
#define EXTRA_COLORS_RED        2
#define EXTRA_COLORS_GREEN      3
#define EXTRA_COLORS_YELLOW     4


#define FIGHT_SPRITE_ROUND_1          0
#define FIGHT_SPRITE_ROUND_2          1
#define FIGHT_SPRITE_ROUND_3          2
#define FIGHT_SPRITE_ROUND_4          3
#define FIGHT_SPRITE_FIGHT_1          4
#define FIGHT_SPRITE_FIGHT_2          5
#define FIGHT_SPRITE_LOSE             6
#define FIGHT_SPRITE_WIN              7
#define FIGHT_SPRITE_DOUBLE_KO        8
#define FIGHT_SPRITE_DRAW             9
#define FIGHT_SPRITE_BADGE           10
#define FIGHT_SPRITE_COUNT           11

#define FIGHT_FRAME_RATE             54000

static const char* sprite_names[] = {
  "round1",
  "round2",
  "round3",
  "rounde",
  "lucha1",
  "lucha2",
  "lose",
  "win",
  "dko",
  "draw",
  "badge",
};

typedef enum fight_status_t_
{
  pre_fight = 0,
  in_fight  = 1,
  post_fight = 2,
  end_fight = 3,
} fight_status_t;

typedef struct fight_state_t_
{
  state_t state;
  fight_status_t status;
  player_input_t input;
  sprite_t* background;
  palette_t palette;
  ai_context_t ai_context;
  player_t* player_1;
  player_t* player_2;
  digits_t* digits;
  int round;
  sprite_t* fight_sprites[FIGHT_SPRITE_COUNT];
  sprite_t* to_render_in_center;
  unsigned int time;
  unsigned long step_start;
  unsigned long start_time;
  unsigned long stop_time;
  bool show_boxes;
  play_midi_t* ambient_sound;
} fight_state_t;

#define FLOOR_Y                     475

static void in_fight_step(fight_state_t* state, const int last_key);
static void pre_fight_step(fight_state_t* state, const int last_key);
static void post_fight_step(fight_state_t* state, const int last_key);
static void end_fight_step(fight_state_t* state, const int last_key);

static void fight_ambient_music(fight_state_t* state)
{
  if(state->ambient_sound)
  {
    if(!play_midi_is_playing(state->ambient_sound))
    {
      play_midi_start(state->ambient_sound);
    }
    play_midi_tick(state->ambient_sound);
  }
}


static void fill_fight_context(fight_state_t* state, fight_context_t* fight_context)
{
  fight_context->fighting = (state->status == in_fight);
  fight_context->floor = FLOOR_Y;
  fight_context->width = sprite_width(state->background);
  fight_context->remaining_time = state->time;
}

static square_t player_to_world(player_t* player, const square_t* square)
{
  const int player_y = player_y_pos(player);
  const int player_x = player_x_pos(player);
  const bool flipped = player_flipped(player);
  square_t result = *square;

  result.y += player_y;
  result.x += player_x;

  if(flipped)
  {
    const int width = sprite_width(player_get_sprite(player));
    result.x = player_x + width - square->sx - square->x;
  }
  return result;
}

static void render_bar(fight_state_t* state, int player_number)
{
  player_t* player = player_number == 1 ? state->player_1 : state->player_2;
  const int x_offset = player_number == 1 ? 10 : 360;
  const int x0 = render_get_view_offset() + x_offset;
  const palette_info_t* palette_info = render_get_palette_info();
  const int sy = 26;
  const int y0 = 15;
  const int sx = 320 - 50;
  const int life = (sx - 4) * player_life(player) / 100;
  const int rest = sx - life - 4;
  const int green_index = palette_info->extra_index + EXTRA_COLORS_GREEN;
  const int red_index = palette_info->extra_index + EXTRA_COLORS_RED;
  const sprite_t* name = player_name(player);
  const int name_sx = sprite_width(name);
  const int name_sy = sprite_height(name);
  const int name_x = player_number == 1 ? x0 + 4 : x0 + sx - 4 - name_sx;
  square_t square = {0};

  square.x = x0;
  square.y = y0;
  square.sx = sx;
  square.sy = sy;
  render_square(&square, palette_info->extra_index + EXTRA_COLORS_BLACK, SQUARE_NO_FILL);

  square.x += 1;
  square.y += 1;
  square.sx -= 2;
  square.sy -= 2;
  render_square(&square, palette_info->extra_index + EXTRA_COLORS_GREY, SQUARE_NO_FILL);

  square.y += 1;
  square.sy -= 2;
  square.x += 1;
  square.sx = life;
  render_square(&square, green_index, green_index);

  square.x += square.sx;
  square.sx = rest;
  render_square(&square, red_index, red_index);

  square.y += 1;
  square.x = name_x;
  square.sx = name_sx;
  square.sy = name_sy;
  render_sprite(name, &square, false);
}

static void render_wins(fight_state_t* state)
{
  int i = 0;
  const int x0 = render_get_view_offset() + 10 + 2;
  const int y0 = 15 + 25 + 5;
  square_t square = {0};
  const sprite_t* badge = state->fight_sprites[FIGHT_SPRITE_BADGE];

  square.x = x0;
  square.y = y0;
  square.sx = sprite_width(badge);
  square.sy = sprite_height(badge);
  for(i = 0; i < player_wins(state->player_1); ++i)
  {
    render_sprite(badge, &square, false);
    square.x += square.sx + 5;
  }

  square.x = render_get_view_offset() + 360 + 320 - 50 - square.sx -2;
  for(i = 0; i < player_wins(state->player_2); ++i)
  {
    render_sprite(badge, &square, false);
    square.x -= square.sx + 5;
  }
}

static void render_scene(fight_state_t* state)
{
  player_t* player_1 = state->player_1;
  player_t* player_2 = state->player_2;
  sprite_t* sprite_1 = player_get_sprite(player_1);
  sprite_t* sprite_2 = player_get_sprite(player_2);
  const unsigned int height_1 = sprite_height(sprite_1);
  const unsigned int width_1 = sprite_width(sprite_1);
  const unsigned int height_2 = sprite_height(sprite_2);
  const unsigned int width_2 = sprite_width(sprite_2);
  square_t square_1 = {0};
  square_t square_2 = {0};
  square_t square_1_shadow = {0};
  square_t square_2_shadow = {0};
  const unsigned int secs_first_digit = state->time / 10;
  const unsigned int secs_second_digit = state->time % 10;
  long unsigned int elapsed_time;
  double elapsed_ms;
  char txt[256];

  square_1.x = player_x_pos(player_1);
  square_1.y = player_y_pos(player_1);
  square_1.sx = width_1;
  square_1.sy = height_1;

  square_1_shadow.x = player_x_pos(player_1);
  square_1_shadow.y = player_y_pos(player_1) + (FLOOR_Y - player_y_pos(player_1)) * 2 / 3;
  square_1_shadow.sx = width_1;
  square_1_shadow.sy = height_1 / 3;


  square_2.x = player_x_pos(player_2);
  square_2.y = player_y_pos(player_2);
  square_2.sx = width_2;
  square_2.sy = height_2;

  square_2_shadow.x = player_x_pos(player_2);
  square_2_shadow.y = player_y_pos(player_2) + (FLOOR_Y - player_y_pos(player_2)) * 2 / 3;
  square_2_shadow.sx = width_2;
  square_2_shadow.sy = height_2 / 3;
  
  render_clean();

  render_shadow(sprite_1, &square_1_shadow, player_flipped(player_1));
  fight_ambient_music(state);
  render_shadow(sprite_2, &square_2_shadow, player_flipped(player_2));
  fight_ambient_music(state);

  if(player_hit(player_2))
  {
    render_sprite(sprite_2, &square_2, player_flipped(player_2));
    fight_ambient_music(state);
    render_sprite(sprite_1, &square_1, player_flipped(player_1));
    fight_ambient_music(state);

  }
  else
  {
    render_sprite(sprite_1, &square_1, player_flipped(player_1));
    fight_ambient_music(state);

    render_sprite(sprite_2, &square_2, player_flipped(player_2));
    fight_ambient_music(state);
  }


  if(state->show_boxes)
  {
    int i = 0;
    player_pose_t* player_1_pose = player_get_pose(player_1);
    player_pose_t* player_2_pose = player_get_pose(player_2);

    for(i = 0; i < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_1_pose->damage_boxes[i])); ++i)
    {
      const square_t box = player_to_world(player_1, &(player_1_pose->damage_boxes[i]));
      render_square(&box, 0, SQUARE_NO_FILL);
    }

    for(i = 0; i < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_1_pose->hit_boxes[i])); ++i)
    {
      const square_t box = player_to_world(player_1, &(player_1_pose->hit_boxes[i]));
      render_square(&box, 0, SQUARE_NO_FILL);
    }

    for(i = 0; i < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_2_pose->damage_boxes[i])); ++i)
    {
      const square_t box = player_to_world(player_2, &(player_2_pose->damage_boxes[i]));
      render_square(&box, 0, SQUARE_NO_FILL);
    }

    for(i = 0; i < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_2_pose->hit_boxes[i])); ++i)
    {
      const square_t box = player_to_world(player_2, &(player_2_pose->hit_boxes[i]));
      render_square(&box, 0, SQUARE_NO_FILL);
    }
  }
  {
    const int y0 = 10;
    const int x0_border = 2;
    const int screen_med = render_get_view_offset() + 320;
    const sprite_t* first_digit = digits_get(state->digits, secs_first_digit);
    const sprite_t* second_digit = digits_get(state->digits, secs_second_digit);
    const int x0 = screen_med - sprite_width(first_digit) - x0_border;
    square_t first_digit_square;
    square_t second_digit_square;

    first_digit_square.x = x0;
    first_digit_square.y = y0;
    first_digit_square.sx = sprite_width(first_digit);
    first_digit_square.sy = sprite_height(first_digit);

    second_digit_square.y = y0;
    second_digit_square.x = screen_med + x0_border;
    second_digit_square.sx = sprite_width(second_digit);
    second_digit_square.sy = sprite_height(second_digit);

    render_sprite(first_digit, &first_digit_square, false);
    render_sprite(second_digit, &second_digit_square, false);
    fight_ambient_music(state);
  }

  // render bars
  render_bar(state, 1);
  fight_ambient_music(state);
  render_bar(state, 2);
  fight_ambient_music(state);

  // render wins
  render_wins(state);
  fight_ambient_music(state);

  // render message in center of screen
  if(state->to_render_in_center)
  {
    const int x0 = render_get_view_offset();
    const int width = sprite_width(state->to_render_in_center);
    const int height = sprite_height(state->to_render_in_center);
    const int x = 320 - width / 2;
    const int y = 240 - height / 2;
    square_t square = {0};

    square.x = x0 + x;
    square.y = y;
    square.sx = width;
    square.sy = height;
    render_sprite(state->to_render_in_center, &square, false); 
    fight_ambient_music(state);
  }

  while(timer_time_since(state->step_start) < FIGHT_FRAME_RATE)
  {
    fight_ambient_music(state);
  }

  render_show(false);
}

static void adjust_sprite_colors(sprite_t* sprite, int delta)
{
  const int digit_width = sprite_width(sprite);
  const int digit_height = sprite_height(sprite);
  unsigned char* buf = (unsigned char*) sprite_first_stride(sprite);
  int y = 0;

  for(; y < digit_height; ++y)
  {
    int x = 0;
    for(; x < digit_width; ++x)
    {
      if(buf[x] != 0)
      {
        buf[x] += delta;
      }
    }
    buf = (unsigned char*) sprite_next_stride(sprite, buf);
  }  
}

static void fight_start_players(fight_state_t* state)
{
  fight_context_t fight_context;
  sprite_t* sprite_1 = NULL;
  sprite_t* sprite_2 = NULL;
  int height_1 = 0;
  int height_2 = 0;

  fill_fight_context(state, &fight_context);

  player_clear_hit(state->player_1);
  player_clear_hit(state->player_2);

  player_standing_enter(&fight_context, state->player_1, state->player_2);
  player_standing_enter(&fight_context, state->player_2, state->player_1);

  sprite_1 = player_get_sprite(state->player_1);
  sprite_2 = player_get_sprite(state->player_2);
  height_1 = sprite_height(sprite_1);
  height_2 = sprite_height(sprite_2);

  state->player_1->x_pos = 50;
  state->player_1->y_pos = FLOOR_Y - height_1;
  state->player_1->flipped = false;

  state->player_2->x_pos = 500;
  state->player_2->y_pos = FLOOR_Y - height_2;
  state->player_2->flipped = true; 

  render_set_view_offset(0);
}

static void fight_start(state_t* state)
{
  palette_info_t palette_info = {0};
  fight_state_t* fight_state = (fight_state_t*)state;
  const unsigned int character_colors = sprite_color_count(fight_state->player_1->poses[0].sprite);
  const unsigned int background_colors = sprite_color_count(fight_state->background);
  const palette_t* background_palette = sprite_palette(fight_state->background);
  const unsigned int background_height = sprite_height(fight_state->background);
  const unsigned int background_width = sprite_width(fight_state->background);
  unsigned char* buf = (unsigned char*) sprite_first_stride(fight_state->background);
  const sprite_t* digit_sprite = digits_get(fight_state->digits, 0);
  const palette_t* extra_palette = sprite_palette(digit_sprite);
  const int extra_colors = sprite_color_count(digit_sprite); 
  int index = 0;
  sprite_t* sprite_1 = NULL;
  sprite_t* sprite_2 = NULL;
  unsigned int height_1 = 0U;
  unsigned int height_2 = 0U;
  square_t square;

  fight_state->palette = *sprite_palette(fight_state->player_1->poses[0].sprite);

  palette_info.foreground_index = 0;
  palette_info.foreground_count = character_colors;

  if(character_colors + background_colors * 2 + extra_colors - 1 < 256)
  {
    sprite_t* name_1 = player_name(fight_state->player_1);
    sprite_t* name_2 = player_name(fight_state->player_2);
    const int previous_colors = character_colors + background_colors * 2;
    int fight_state_extra_index = previous_colors;

    // set palette info
    palette_info.background_index = character_colors;
    palette_info.background_count = background_colors;
    palette_info.background_shadow_index = character_colors + background_colors;
    palette_info.background_shadow_count = background_colors;
    palette_info.extra_index = palette_info.background_shadow_index + background_colors;
    palette_info.extra_count = extra_colors;

    // background and character palette colors are merged
    // background colors are doubled to support shadow creation
    while(index < background_colors)
    {
      fight_state->palette.colors[character_colors + index] = background_palette->colors[index];
      fight_state->palette.colors[character_colors + background_colors + index] = background_palette->colors[index];

      if(fight_state->palette.colors[character_colors + background_colors + index].blue >= 75)
      {
        fight_state->palette.colors[character_colors + background_colors + index].blue -= 75;
      }
      else
      {
        fight_state->palette.colors[character_colors + background_colors + index].blue = 0;
      }

      if(fight_state->palette.colors[character_colors + background_colors + index].red >= 75)
      {
        fight_state->palette.colors[character_colors + background_colors + index].red -= 75;
      }
      else
      {
        fight_state->palette.colors[character_colors + background_colors + index].red = 0;
      }

      if(fight_state->palette.colors[character_colors + background_colors + index].green >= 75)
      {
        fight_state->palette.colors[character_colors + background_colors + index].green -= 75;
      }
      else
      {
        fight_state->palette.colors[character_colors + background_colors + index].green = 0;
      }
      ++index;
    }

    // extra colors are merged to the fight state palette
    for(index = 1; index < extra_colors; ++index)
    {
      fight_state->palette.colors[fight_state_extra_index++] = extra_palette->colors[index];
    }

    // adjust background color indexes
    for(index = 0; index < background_height; ++index)
    {
      int j = 0;
      for(;j < background_width; ++j)
      {
          buf[j] += character_colors;
      }
      buf = (unsigned char*) sprite_next_stride(fight_state->background, buf);
    }

    // adjust digits colors
    for(index = 0; index < 10; ++index)
    {
      const sprite_t* digit = digits_get(fight_state->digits, index);
      adjust_sprite_colors((sprite_t*)digit, previous_colors - 1);
    }

    // adjust fight state sprite colors
    for(index = 0; index < FIGHT_SPRITE_COUNT; ++index)
    {
      adjust_sprite_colors(fight_state->fight_sprites[index], previous_colors - 1);
    }

    if(name_1 != NULL)
    {
      adjust_sprite_colors(name_1, previous_colors - 1);
    }

    if(name_2 != NULL && name_2 != name_1)
    {
      adjust_sprite_colors(name_2, previous_colors - 1);
    }
  }

  fight_start_players(fight_state);
  fight_state->round = -1;

  if(fight_state->ambient_sound)
  {
    play_midi_start(fight_state->ambient_sound);
  }

  render_set_background(fight_state->background);
  render_set_palette(&(fight_state->palette));
  render_set_palette_info(&palette_info);

  render_scene(fight_state);
}

static void fight_stop(state_t* state)
{
    fight_state_t* fight_state = (fight_state_t*)state;
    timer_read(&fight_state->stop_time);
    play_midi_stop(fight_state->ambient_sound);
}

static void fight_resume(state_t* state)
{
  fight_state_t* fight_state = (fight_state_t*)state;
  fight_state->start_time += timer_time_since(fight_state->stop_time);
  play_midi_resume(fight_state->ambient_sound);
}

static void check_scenario_limits(fight_state_t* fight_state)
{
  player_t* player_1 = fight_state->player_1;
  player_t* player_2 = fight_state->player_2;
  const sprite_t* sprite_1 = player_get_sprite(player_1);
  const sprite_t* sprite_2 = player_get_sprite(player_2);
  const int sprite_1_width = sprite_width(sprite_1);
  const int sprite_2_width = sprite_width(sprite_2);
  const int sprite_1_height = sprite_height(sprite_1);
  const int sprite_2_height = sprite_height(sprite_2);
  const int background_width = sprite_width(fight_state->background);

  if(player_1->x_pos < 0)
  {
    player_1->x_pos = 0;
  }
  else if(player_1->x_pos + sprite_1_width > background_width)
  {
    player_1->x_pos = background_width - sprite_1_width;
  }

  if(player_1->y_pos < 0)
  {
    player_1->y_pos = 0;
  }
  else if(player_1->y_pos + sprite_1_height > FLOOR_Y)
  {
    player_1->y_pos = FLOOR_Y - sprite_1_height;
  }

  if(player_2->x_pos < 0)
  {
    player_2->x_pos = 0;
  }
  else if(player_2->x_pos + sprite_2_width > background_width)
  {
    player_2->x_pos = background_width - sprite_2_width;
  }

  if(player_2->y_pos < 0)
  {
    player_2->y_pos = 0;
  }
  else if(player_2->y_pos + sprite_2_height > FLOOR_Y)
  {
    player_2->y_pos = FLOOR_Y - sprite_2_height;
  }
}

static player_t* get_player_on_left_side(const fight_state_t* fight_state)
{
  player_t* player_1 = fight_state->player_1;
  player_t* player_2 = fight_state->player_2;
  const int player_1_x = player_x_pos(player_1);
  const int player_2_x = player_x_pos(player_2);
  return player_1_x < player_2_x ? player_1 : player_2;
}

static player_t* get_player_on_right_side(const fight_state_t* fight_state)
{
  player_t* player_1 = fight_state->player_1;
  player_t* player_2 = fight_state->player_2;
  const int player_1_x = player_x_pos(player_1);
  const int player_2_x = player_x_pos(player_2);
  return player_1_x >= player_2_x ? player_1 : player_2;
}


static void check_screen_limits(fight_state_t* fight_state)
{
  player_t* left_player = get_player_on_left_side(fight_state);
  player_t* right_player = get_player_on_right_side(fight_state);
  const sprite_t* sprite_right_player = player_get_sprite(right_player);
  const int min_x = player_x_pos(left_player);
  const int player_right_pos = player_x_pos(right_player);
  const int sprite_right_width = sprite_width(sprite_right_player);
  const int max_x = player_right_pos + sprite_right_width;
  const int distance = max_x - min_x;
  if(distance > 640)
  {
    if(player_status(left_player) == STATUS_STANDING)
    {
      player_set_x_pos(right_player, min_x + 640 - sprite_right_width);
    } 
    else if(player_status(right_player) == STATUS_STANDING)
    {
      player_set_x_pos(left_player, max_x - 640);
    }
    else
    {
       const int diff = (distance - 640) / 2;
       player_set_x_pos(left_player, min_x + diff);
       player_set_x_pos(right_player, player_right_pos - diff);
    }
  }
}

static void check_collisions(fight_state_t* fight_state)
{
  player_t* player_left = get_player_on_left_side(fight_state);
  player_t* player_right = get_player_on_right_side(fight_state);
  const int player_left_x = player_x_pos(player_left);
  const int player_left_y = player_y_pos(player_left);
  const int player_right_x = player_x_pos(player_right);
  const int player_right_y = player_y_pos(player_right);

  const sprite_t* sprite_left = player_get_sprite(player_left);
  const sprite_t* sprite_right = player_get_sprite(player_right);
  const int sprite_left_width = sprite_width(sprite_left);
  const int sprite_left_height = sprite_height(sprite_left);
  const int sprite_right_width = sprite_width(sprite_right);
  const int sprite_right_height = sprite_height(sprite_right);

  square_t player_left_square = {0};
  square_t player_right_square = {0};
  square_t intersection =  {0};

  player_left_square.x = player_left_x;
  player_left_square.y = player_left_y;
  player_left_square.sx = sprite_left_width;
  player_left_square.sy = sprite_left_height;

  player_right_square.x = player_right_x;
  player_right_square.y = player_right_y;
  player_right_square.sx = sprite_right_width;
  player_right_square.sy = sprite_right_height;

  intersection = square_intersection(&player_left_square, &player_right_square);

  if(!square_empty(&intersection))
  {
    int left_delta = intersection.sx / 2;
    int right_delta = left_delta;
    int new_left_x = player_left_x;
    int new_right_x = player_right_x;
    
    new_left_x = player_left_x - left_delta;
    new_right_x = player_right_x + right_delta;

    if(new_left_x < 0)
    {
      new_right_x -= new_left_x;
      new_left_x = 0;
    }
    else
    {
      const int background_width = sprite_width(fight_state->background);
      const int right_end = new_right_x + sprite_right_width;
      if(right_end > background_width)
      {
        const int diff = right_end - background_width;
        new_right_x -= diff;
        new_left_x -= diff;
      }
    }

    player_set_x_pos(player_left, new_left_x);
    player_set_x_pos(player_right, new_right_x);
  }
}

static unsigned int update_view_offset(fight_state_t* fight_state)
{
  player_t* player_1 = get_player_on_left_side(fight_state);
  player_t* player_2 = get_player_on_right_side(fight_state);
  const sprite_t* sprite_1 = player_get_sprite(player_1);
  const sprite_t* sprite_2 = player_get_sprite(player_2);
  const int min_x = player_1->x_pos;
  const int max_x = player_2->x_pos + sprite_width(sprite_2);
  const int background_width = sprite_width(fight_state->background);
  unsigned int view_offset = render_get_view_offset();
  unsigned int old_view_offset = view_offset;
  const unsigned int half_x = min_x + (max_x - min_x) / 2;

  if(half_x < 320)
  {
    view_offset = 0;
  } 
  else
  {
    view_offset = half_x - 320;
  }
  
  if(view_offset + 640 > background_width)
  {
    view_offset = background_width - 640;
  }

  if(view_offset > old_view_offset)
  {
      const int diff = view_offset - old_view_offset; 
      view_offset = old_view_offset + min(diff, 5);
  }
  else if(view_offset < old_view_offset)
  {
      const int diff = old_view_offset - view_offset; 
      view_offset = old_view_offset - min(diff, 5);
  }  
  return view_offset;
}

static bool check_hit(player_t* player_1, player_t* player_2)
{
  const player_pose_t* player_pose_1 = player_get_pose(player_1);
  const player_pose_t* player_pose_2 = player_get_pose(player_2);  
  const int player_1_x = player_x_pos(player_1);
  const int player_1_y = player_y_pos(player_1);
  const int pose_1_width = sprite_width(player_pose_1->sprite);
  const int pose_2_width = sprite_width(player_pose_2->sprite);
  const int player_2_x = player_x_pos(player_2);
  const int player_2_y = player_y_pos(player_2);
  const bool player_1_flipped = player_flipped(player_1);
  const bool player_2_flipped = player_flipped(player_2);

  bool result = false;

  int i = 0;
  for(; i < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_pose_1->hit_boxes[i])) && !result ; ++i)
  {
    int j = 0;
    const square_t hit_box = player_to_world(player_1, &(player_pose_1->hit_boxes[i]));
 
    for(; j < PLAYER_POSE_MAX_BOXES && !square_empty(&(player_pose_2->damage_boxes[j])); ++j)
    {
      const square_t damage_box = player_to_world(player_2, &(player_pose_2->damage_boxes[j]));
      const square_t intersect = square_intersection(&hit_box, &damage_box);
      if(!square_empty(&intersect))
      {
        const int x_hit = player_1_flipped ? hit_box.x : hit_box.x + hit_box.sx;
        const int y_hit = hit_box.y;
        result = true;
        player_set_hit(player_2, x_hit, y_hit, player_status(player_1));
        break;
      }
    }
  }
  return result;
}

static bool check_hits(fight_state_t* fight_state)
{
  const bool p2_hit = check_hit(fight_state->player_1, fight_state->player_2);
  const bool p1_hit = check_hit(fight_state->player_2, fight_state->player_1);
  return p2_hit || p1_hit;
}

static void adjust_positions(fight_state_t* fight_state)
{
  unsigned int view_offset = 0;

  if(check_hits(fight_state))
  {
    if(!play_wav_busy())
    {
      play_wav_start(PLAY_WAV_HIT);
    }
  }
  check_collisions(fight_state);

  check_scenario_limits(fight_state);
  check_screen_limits(fight_state);

  view_offset = update_view_offset(fight_state);
  render_set_view_offset(view_offset);
}

static void update_input(player_input_t* input, const int last_key)
{
  switch(last_key)
  {
    case KEY_CONFIG_UP:
      player_input_press_key(input, PLAYER_INPUT_UP);
      break;
    case KEY_CONFIG_RIGHT:
      player_input_press_key(input, PLAYER_INPUT_RIGHT);
      break;
    case KEY_CONFIG_LEFT:
      player_input_press_key(input, PLAYER_INPUT_LEFT);
      break;
    case KEY_CONFIG_DOWN:
      player_input_press_key(input, PLAYER_INPUT_DOWN);
      break;
    case KEY_CONFIG_HIGH_PUNCH:
      player_input_press_key(input, PLAYER_INPUT_HIGH_PUNCH);
      break;
    case KEY_CONFIG_LOW_PUNCH:
      player_input_press_key(input, PLAYER_INPUT_LOW_PUNCH);
      break;
    case KEY_CONFIG_HIGH_KICK:
      player_input_press_key(input, PLAYER_INPUT_HIGH_KICK);
      break;
    case KEY_CONFIG_LOW_KICK:
      player_input_press_key(input, PLAYER_INPUT_LOW_KICK);
      break;
    case KEY_CONFIG_DEFENSE:
      player_input_press_key(input, PLAYER_INPUT_DEFENSE);
      break;
    case KEY_CONFIG_UP_END:
      player_input_release_key(input, PLAYER_INPUT_UP);
      break;
    case KEY_CONFIG_RIGHT_END:
      player_input_release_key(input, PLAYER_INPUT_RIGHT);
      break;
    case KEY_CONFIG_LEFT_END:
      player_input_release_key(input, PLAYER_INPUT_LEFT);
      break;
    case KEY_CONFIG_DOWN_END:
      player_input_release_key(input, PLAYER_INPUT_DOWN);
      break;
    case KEY_CONFIG_HIGH_PUNCH_END:
      player_input_release_key(input, PLAYER_INPUT_HIGH_PUNCH);
      break;
    case KEY_CONFIG_LOW_PUNCH_END:
      player_input_release_key(input, PLAYER_INPUT_LOW_PUNCH);
      break;
    case KEY_CONFIG_HIGH_KICK_END:
      player_input_release_key(input, PLAYER_INPUT_HIGH_KICK);
      break;
    case KEY_CONFIG_LOW_KICK_END:
      player_input_release_key(input, PLAYER_INPUT_LOW_KICK);
      break;
    case KEY_CONFIG_DEFENSE_END:
      player_input_release_key(input, PLAYER_INPUT_DEFENSE);
      break;
  }
}

static void end_fight_step(fight_state_t* state, const int last_key)
{
}

static void post_fight_step(fight_state_t* state, const int last_key)
{
  fight_context_t fight_context;
  double elapsed_seconds = 0.0;
  int life = 0;

  fill_fight_context(state, &fight_context);
  if(state->start_time == 0)
  {
    timer_read(&state->start_time);
    player_clear_input(state->player_1);
    player_clear_input(state->player_2);
  }
  else
  {
    const unsigned long long micro_seconds = timer_time_since(state->start_time);
    elapsed_seconds = (double) micro_seconds / 1000000;
  }

  player_advance(&fight_context, state->player_1, state->player_2);
  player_advance(&fight_context, state->player_2, state->player_1);  
  adjust_positions(state);  

  if(elapsed_seconds < 3)
  {
    update_input(&state->input, last_key);
    if(state->to_render_in_center == NULL)
    {
      const int player_1_status = player_status(state->player_1);
      const int player_2_status = player_status(state->player_2);

      if(player_1_status == STATUS_WIN)
      {
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_WIN];
      }
      else if(player_2_status == STATUS_WIN)
      {
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_LOSE];
      }
      else if(player_1_status == STATUS_KO
      && player_2_status == STATUS_KO)
      {
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_DOUBLE_KO];
      }
      else if(player_1_status == STATUS_STANDING
      && player_2_status == STATUS_STANDING)
      {
        fight_context_t fight_context;
        const int player_1_life = player_life(state->player_1);
        const int player_2_life = player_life(state->player_2);

        fill_fight_context(state, &fight_context);
        if(player_1_life > player_2_life)
        {
          player_win_enter(&fight_context, state->player_1, state->player_2);
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_WIN];
        }
        else if(player_1_life < player_2_life)
        {
          player_win_enter(&fight_context, state->player_2, state->player_1);
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_LOSE];
        }
        else
        {
          state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_DRAW];
        }
      }
    }
  }
  else
  {
    int new_win_count = 0;
    bool exit = false;

    if(player_status(state->player_1) == STATUS_WIN)
    {
      new_win_count = player_wins(state->player_1) + 1;
      player_set_wins(state->player_1, new_win_count);
    }
    else if(player_status(state->player_2) == STATUS_WIN)
    {
      new_win_count = player_wins(state->player_2) + 1;
      player_set_wins(state->player_2, new_win_count);
    }

    if (new_win_count == 2)
    {
      state->start_time = 0;
      state->to_render_in_center = NULL;
      state->status = end_fight;
      end_fight_step(state, last_key);     
    }
    else
    {
      state->start_time = 0;
      state->to_render_in_center = NULL;
      state->status = pre_fight;
      pre_fight_step(state, last_key);
    }
  }
}

static void in_fight_step(fight_state_t* state, const int last_key)
{
  if(state->start_time == 0)
  {
    timer_read(&state->start_time);
    ai_init(&state->ai_context);
  }

  if(state->time == 0
  || player_status(state->player_1) == STATUS_KO
  || player_status(state->player_2) == STATUS_KO)
  {
    state->start_time = 0;
    state->status = post_fight;
    post_fight_step(state, last_key);
  }
  else
  {
    fight_context_t fight_context;
    int player_2_key = KEY_CONFIG_NONE;
    const unsigned long long elapsed_time = timer_time_since(state->start_time);
    const unsigned long long elapsed_seconds = elapsed_time /  1000000;
    const unsigned int secs = max(60 - elapsed_seconds, 0);

    fill_fight_context(state, &fight_context);
    state->time = secs;

    update_input(&state->input, last_key);
    update_input(player_input(state->player_1), last_key);

   player_2_key = ai_last_key(&state->ai_context,&fight_context, state->player_2, state->player_1);
   update_input(player_input(state->player_2), player_2_key);

    player_advance(&fight_context, state->player_1, state->player_2);
    player_advance(&fight_context, state->player_2, state->player_1);

    adjust_positions(state);  
  }
}

static void pre_fight_step(fight_state_t* state, const int last_key)
{
  fight_context_t fight_context;
  double elapsed_seconds = 0.0;
  int life = 0;

  if(state->start_time == 0)
  {
    state->time = 60;
    state->round = min(state->round + 1, FIGHT_SPRITE_ROUND_4);
    fight_start_players(state);
    timer_read(&state->start_time);
  }
  else
  {
    const unsigned long long micro_seconds = timer_time_since(state->start_time);
    elapsed_seconds = (double) micro_seconds / 1000000;
  }

  fill_fight_context(state, &fight_context);

  life = min(100, elapsed_seconds * 100 / 2);

  player_set_life(state->player_1, life);
  player_set_life(state->player_2, life);

  player_advance(&fight_context, state->player_1, state->player_2);
  player_advance(&fight_context, state->player_2, state->player_1);  

  if(life < 100)
  {
    update_input(&state->input, last_key);
    if(elapsed_seconds < 1)
    {
      state->to_render_in_center = state->fight_sprites[state->round];
    }
    else
    {
      if(state->to_render_in_center == state->fight_sprites[state->round])
      {
        play_wav_start(PLAY_WAV_FIGHT);
      }

      if(state->to_render_in_center != state->fight_sprites[FIGHT_SPRITE_FIGHT_1])
      {
        state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_FIGHT_1];
      }
      else
      {
        state->to_render_in_center = state->fight_sprites[FIGHT_SPRITE_FIGHT_2];
      }
    }
  }
  else
  {
    player_set_input(state->player_1, &state->input);
    state->to_render_in_center = NULL;
    state->status = in_fight;
    state->start_time = 0;
    in_fight_step(state, last_key);
  }
}

static bool fight_step(state_t* state)
{
  const int last_key = keyboard_get_key();
  fight_state_t* fight_state = (fight_state_t*)state;
  bool result = true;

  timer_read(&fight_state->step_start);

  if(last_key == KEY_CONFIG_PAUSE)
  {
      state_ctrl_set(game_state_pause);
  }
  else if(last_key == KEY_CONFIG_EXIT)
  {
      result = false;
  }
  else
  {
    fight_ambient_music(fight_state);
    switch(fight_state->status)
    {
      case pre_fight:
        pre_fight_step(fight_state, last_key);
        break;
      case in_fight:
        in_fight_step(fight_state, last_key);
        break;
      case post_fight:
        post_fight_step(fight_state, last_key);
        break;
      case end_fight:
        result = false;
        break;
    }

    render_scene(fight_state);
  }
  return result;
}

void fight_free(state_t* state)
{
  fight_state_t* fight_state = (fight_state_t*)state;
  if(fight_state)
  {
    int i = 0;

    if(fight_state->background)
    {
      sprite_free(fight_state->background);
    }

    if(fight_state->player_1)
    {
      player_free(fight_state->player_1);
    }

    if(fight_state->player_2)
    {
      player_free(fight_state->player_2);
    }

    if(fight_state->digits)
    {
      digits_free(fight_state->digits);
    }

    for(i = 0; i < FIGHT_SPRITE_COUNT; ++i)
    {
      if(fight_state->fight_sprites[i] != NULL)
      {
        sprite_free(fight_state->fight_sprites[i]);

      }
    }

    if(fight_state->ambient_sound)
    {
      play_midi_stop(fight_state->ambient_sound);
      play_midi_close(fight_state->ambient_sound);
    }
  
    free(fight_state);
  }
}

state_t* state_fight_create()
{
  fight_state_t* result = (fight_state_t*) malloc(sizeof(fight_state_t));
  if(result)
  {
    char file_path[256];
    int i = 0;
    int palette_delta = 0;

    memset(result, 0, sizeof(*result));

    result->state.free = fight_free;
    result->state.resume = fight_resume;
    result->state.start = fight_start;
    result->state.step = fight_step;
    result->state.stop = fight_stop;


    // result->show_boxes = true;

    result->background = create_bmp_sprite("assets\\fight\\class.bmp");
    palette_delta = sprite_color_count(result->background);

    result->player_1 = player_create("monika");
    result->player_2 = player_create("mates");

    result->digits = digits_create("assets\\fight\\digits");

    for(i = 0; i < FIGHT_SPRITE_COUNT; ++i)
    {
      sprintf(file_path, "assets\\fight\\%s.bmp", sprite_names[i]);
      result->fight_sprites[i] = create_bmp_sprite(file_path);
      if(!result->fight_sprites[i])
      {
        break;
      }
    }

    result->ambient_sound = play_midi_open("assets\\sounds\\fight.mid");
    if(!result->ambient_sound)
    {
      log_record("Error loading ambient music");
    }

    if( i != FIGHT_SPRITE_COUNT || 
       result->background  == NULL ||
       result->player_1 == NULL ||
       result->player_2 == NULL ||
       result->digits == NULL)
    {
        fight_free(&(result->state));
        result = NULL;
    }
  }
  return result == NULL ? NULL : &(result->state);
}
