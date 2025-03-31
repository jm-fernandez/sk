#include <stddef.h>

#include "player_standing.h"
#include "player.h"

#include "player_jump.h"
#include "player_kick.h"
#include "player_punch.h"
#include "player_walk.h"
#include "player_down.h"
#include "player_hit.h"
#include "player_defense.h"
#include "player_win.h"

#include "sprite.h"

#define STATUS_STANDING_STATE_1     1
#define STATUS_STANDING_STATE_2     2

static int standing_poses[] = {
  POSE_STANDING_1,
  POSE_STANDING_1,
  POSE_STANDING_1,
  POSE_STANDING_2,
  POSE_STANDING_2,
  POSE_STANDING_2,
  POSE_STANDING_3,
  POSE_STANDING_3,
  POSE_STANDING_3,
  POSE_STANDING_4,
  POSE_STANDING_4,
  POSE_STANDING_4,
  -1,
};

static void player_standing_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  bool leave = false;
  
  player->flipped = (enemy->x_pos < player->x_pos);
  
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(enemy->status == STATUS_KO)
  {
    player_win_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_DEFENSE))
  {
    player_defense_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK))
  {
    if(player_is_key_pressed(player, PLAYER_INPUT_LEFT) && !player->flipped)
    {
      player_outward_enter(fight_context, player, enemy);
    }
    else if(player_is_key_pressed(player, PLAYER_INPUT_RIGHT) && player->flipped)
    {
      player_outward_enter(fight_context, player, enemy);
    }
    else
    {
      player_high_kick_enter(fight_context, player, enemy);
    }
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))
  {
    if(player_is_key_pressed(player, PLAYER_INPUT_LEFT) && !player->flipped)
    {
      player_sweep_enter(fight_context, player, enemy);
    }
    else if(player_is_key_pressed(player, PLAYER_INPUT_RIGHT) && player->flipped)
    {
      player_sweep_enter(fight_context, player, enemy);
    }
    else
    {
      player_low_kick_enter(fight_context, player, enemy);
    }
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LEFT))
  {
      if(player->flipped)
      {
        player_walk_forward_enter(fight_context, player, enemy);
      }
      else
      {
        player_walk_backward_enter(fight_context, player, enemy);
      }
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_RIGHT))
  {
      if(player->flipped)
      {
        player_walk_backward_enter(fight_context, player, enemy);
      }
      else
      {
        player_walk_forward_enter(fight_context, player, enemy);
      }
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_UP))
  {
    player_jump_vertical_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH))
  {
      player_high_punch_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_PUNCH))
  {
      player_low_punch_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_DOWN))
  {
      player_down_enter(fight_context, player, enemy);
  }
  else
  {
      ++player->status2;
      if(standing_poses[player->status2] == -1)
      {
        player->status2 = 0;
      }
      player_set_pose(player, &player->poses[standing_poses[player->status2]], player_pose_left_aligned);
  }
}

void player_standing_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const int y_pos = fight_context->floor - sprite_height(player->poses[POSE_STANDING_1].sprite);
  int adjust_x_position = 0;
  if(player->current_pose && player->flipped)
  {
      const int current_sprite_width = sprite_width(player->current_pose->sprite);  
      const int standing_sprite_width =  sprite_width(player->poses[POSE_STANDING_1].sprite);
      const int current_x_end = player->x_pos + current_sprite_width;
      const int new_x = current_x_end - standing_sprite_width;
      adjust_x_position = new_x - player->x_pos; 
  }

  player->status = STATUS_STANDING;
  player->status2 = 0;
  player->advance = player_standing_advance;

  player->current_pose = &(player->poses[POSE_STANDING_1]);
  player->y_pos = y_pos;
  player->x_pos += adjust_x_position;
}
