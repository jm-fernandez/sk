#include "player_kick.h"
#include "player.h"
#include "player_standing.h"
#include "player_down.h"
#include "player_hit.h"

static const int high_kick_poses[] = {
  POSE_KICK_1,
  POSE_KICK_2,
  POSE_KICK_3,
  POSE_KICK_3,
  POSE_KICK_2,
  -1,
};

static const int low_kick_poses[] = {
  POSE_KICK_1,
  POSE_KICK_2,
  POSE_KICK_2_L,
  POSE_KICK_2,
  -1,
};

static const int outward_poses[] = {
  POSE_OUTWARD_1,
  POSE_OUTWARD_2,
  POSE_OUTWARD_3,
  POSE_OUTWARD_3,
  POSE_OUTWARD_4,
  POSE_OUTWARD_4,
  POSE_OUTWARD_5,
  POSE_OUTWARD_6,
  -1,
};

static const int down_low_kick_poses[] = {
  POSE_DOWN_KICK_1,
  POSE_DOWN_KICK_L_2,  
  POSE_DOWN_KICK_L_2,  
  POSE_DOWN_KICK_1,
  -1,
};

static const int down_high_kick_poses[] = {
  POSE_DOWN_KICK_1,
  POSE_DOWN_KICK_2,
  POSE_DOWN_KICK_3,
  POSE_DOWN_KICK_3,
  POSE_DOWN_KICK_2,
  POSE_DOWN_KICK_1,
  -1,
};

static const int sweep_poses[] = {
  POSE_LSWEEP_1,
  POSE_LSWEEP_1,
  POSE_LSWEEP_2,
  POSE_LSWEEP_2,
  POSE_LSWEEP_3,
  POSE_LSWEEP_3,
  POSE_LSWEEP_4,
  POSE_LSWEEP_4,
  POSE_LSWEEP_5,
  POSE_LSWEEP_5,
  -1,
};

void player_kick_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
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

void player_low_kick_enter(const fight_context_t* fight_context,player_t* player, player_t* enemy)
{
  player->status = STATUS_LOW_KICK;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = low_kick_poses;
  player_kick_advance(fight_context, player, enemy);
}

void player_high_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_HIGH_KICK;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = high_kick_poses;
  player_kick_advance(fight_context, player, enemy);
}

void player_outward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_OUTWARD;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = outward_poses;
  player_kick_advance(fight_context, player, enemy);  
}

void player_down_low_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_DOWN_LOW_KICK;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = down_low_kick_poses;
  player_kick_advance(fight_context, player, enemy);

}

void player_down_high_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_DOWN_HIGH_KICK;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = down_high_kick_poses;
  player_kick_advance(fight_context, player, enemy);
}

void player_sweep_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_SWEEP;
  player->advance = player_kick_advance;
  player->status_pos = -1;
  player->status_list = sweep_poses;
  player_kick_advance(fight_context, player, enemy);
}
