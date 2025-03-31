#include <stddef.h>

#include "player_punch.h"
#include "player.h"

#include "player_standing.h"
#include "player_hit.h"
#include "player_down.h"

#define STATUS_PUNCH_STATE_1        1
#define STATUS_PUNCH_STATE_2        2
#define STATUS_PUNCH_STATE_3        3
#define STATUS_PUNCH_STATE_4        4
#define STATUS_PUNCH_STATE_5        5
#define STATUS_PUNCH_STATE_6        6
#define STATUS_PUNCH_STATE_7        7
#define STATUS_PUNCH_STATE_8        8

static const int poses_down_punch [] = {
  POSE_DOWN_PUNCH_1,
  POSE_DOWN_PUNCH_2,
  POSE_DOWN_PUNCH_2,
  POSE_DOWN_PUNCH_1,
  -1,
};

static player_pose_t* player_punch_get_pose(player_t* player)
{
  player_pose_t* result = NULL;
  switch(player->status2)
  {
    case STATUS_PUNCH_STATE_1:
      result = &(player->poses[POSE_PUNCH_1]);
      break;
    case STATUS_PUNCH_STATE_2:
    case STATUS_PUNCH_STATE_8:
      result = &(player->poses[POSE_PUNCH_2]);
      break;
    case STATUS_PUNCH_STATE_3:
      if(player->status == STATUS_HIGH_PUNCH)
      {
        result = &(player->poses[POSE_PUNCH_3]);
      }
      else
      {
        result = &(player->poses[POSE_PUNCH_3L]);
      }
      break;
    case STATUS_PUNCH_STATE_4:
      result = &(player->poses[POSE_PUNCH_4]);
      break;
    case STATUS_PUNCH_STATE_5:
    case STATUS_PUNCH_STATE_7:
      result = &(player->poses[POSE_PUNCH_5]);
      break;
    case STATUS_PUNCH_STATE_6:
      if(player->status == STATUS_HIGH_PUNCH)
      {
        result = &(player->poses[POSE_PUNCH_6]);
      }
      else
      {
        result = &(player->poses[POSE_PUNCH_6L]);
      }
      break;
  }
  return result;
}

static void player_punch_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else
  {
    const bool high_punch_pressed = player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH);
    
    switch(player->status2)
    {
      case STATUS_PUNCH_STATE_3:
        player->status2 = (high_punch_pressed) ? STATUS_PUNCH_STATE_4 : STATUS_PUNCH_STATE_8;
        player_set_pose(player, player_punch_get_pose(player), player_pose_left_aligned);
        break;
      case STATUS_PUNCH_STATE_7:
        player->status2 = (high_punch_pressed) ? STATUS_PUNCH_STATE_4 : STATUS_PUNCH_STATE_1;
        player_set_pose(player, player_punch_get_pose(player), player_pose_left_aligned);
        break;
      case STATUS_PUNCH_STATE_8:
        if(high_punch_pressed)
        {
          player->status2 = STATUS_PUNCH_STATE_3;
          player_set_pose(player, player_punch_get_pose(player), player_pose_left_aligned);
        }
        else
        {
          player_standing_enter(fight_context, player, enemy);
        }
        break;
      default:
        ++player->status2;
        player_set_pose(player, player_punch_get_pose(player), player_pose_left_aligned);
    }
  }
}

void player_high_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_HIGH_PUNCH;
  player->status2 = STATUS_PUNCH_STATE_1;
  player->advance = player_punch_advance;
  player_set_pose(player, &(player->poses[POSE_PUNCH_1]), player_pose_left_aligned);
}

void player_low_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_LOW_PUNCH;
  player->status2 = STATUS_PUNCH_STATE_1;
  player->advance = player_punch_advance;
  player_set_pose(player, &(player->poses[POSE_PUNCH_1]), player_pose_left_aligned);
}

void player_down_punch_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else
  {
    ++player->status_pos;
    if(player->status_list[player->status_pos] == -1)
    {
      if(player_is_key_pressed(player, PLAYER_INPUT_DOWN))
      {
        player_down_enter(fight_context, player, enemy);
      }
      else
      {
        player_standing_enter(fight_context, player, enemy);    
      }
    }
    else
    {
      const int pose_idx = player->status_list[player->status_pos];
      player_set_pose(player, &player->poses[pose_idx], player_pose_left_aligned);
    }
  }
}

void player_down_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_DOWN_PUNCH;
  player->advance = player_down_punch_advance;
  player->status_pos = -1;
  player->status_list = poses_down_punch;
  player_down_punch_advance(fight_context, player, enemy);
}
