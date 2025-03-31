#include <stddef.h>

#include "player_walk.h"
#include "player.h"
#include "player_punch.h"
#include "player_standing.h"
#include "player_jump.h"
#include "player_kick.h"
#include "player_hit.h"

#define STATUS_WALK_STATE_1         0
#define STATUS_WALK_STATE_2         1
#define STATUS_WALK_STATE_3         2
#define STATUS_WALK_STATE_4         3
#define STATUS_WALK_STATE_5         4
#define STATUS_WALK_STATE_6         5

static void player_walk_forward_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const bool flipped = player->flipped;
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK))
  {
    player_high_kick_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))
  {
    player_low_kick_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH))
  {
    player_high_punch_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_UP))
  {
    player_jump_forward_enter(fight_context, player, enemy);
  }
  else if((!flipped && !player_is_key_pressed(player, PLAYER_INPUT_RIGHT))
  || (flipped && !player_is_key_pressed(player, PLAYER_INPUT_LEFT)))
  {
    player_standing_enter(fight_context, player, enemy);
  }
  else
  {
    if(player->flipped)
    {
      player->x_pos -= 15;
    }
    else
    {
      player->x_pos += 15;
    }

    if(player->status2 == STATUS_WALK_STATE_6)
    {
      player->status2 = STATUS_WALK_STATE_1;
    }
    ++player->status2;

    player_set_pose(player, &(player->poses[POSE_WALK_1 + player->status2]), player_pose_left_aligned);
  }
}

static void player_walk_backward_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const bool flipped = player->flipped;

  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK))
  {
    player_outward_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))
  {
    player_sweep_enter(fight_context, player, enemy);
  }  
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH))
  {
    player_high_punch_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_UP))
  {
    player_jump_backward_enter(fight_context, player, enemy);
  }
  else if((!flipped && !player_is_key_pressed(player, PLAYER_INPUT_LEFT))
  || (flipped && !player_is_key_pressed(player, PLAYER_INPUT_RIGHT)))
  {
    player_standing_enter(fight_context, player, enemy);
  }
  else
  {
    if(flipped)
    {
      player->x_pos += 15;
    }
    else
    {
      player->x_pos -= 15;
    }

    if(player->status2 == STATUS_WALK_STATE_1)
    {
      player->status2 = STATUS_WALK_STATE_6;
    }    
    --player->status2;

    player_set_pose(player, &(player->poses[POSE_WALK_1 + player->status2]), player_pose_left_aligned);
  }
}

void player_walk_forward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_WALK_FORWARD;
  player->status2 = STATUS_WALK_STATE_1;
  player->advance = player_walk_forward_advance;
  player_set_pose(player, &(player->poses[POSE_WALK_1]), player_pose_left_aligned);
}

void player_walk_backward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_WALK_BACKWARD;
  player->status2 = STATUS_WALK_STATE_6;
  player->advance = player_walk_backward_advance;
  player_set_pose(player, &(player->poses[POSE_WALK_6]), player_pose_left_aligned);
}
