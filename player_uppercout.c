#include "player.h"
#include "player_uppercout.h"
#include "player_standing.h"
#include "player_hit.h"

static int uppercut_poses[] = {
  POSE_UPPERCUT_1,
  POSE_UPPERCUT_1,
  POSE_UPPERCUT_2,
  POSE_UPPERCUT_2,
  POSE_UPPERCUT_3,
  POSE_UPPERCUT_3,
  POSE_UPPERCUT_4,
  POSE_UPPERCUT_4,
  -1,
};

static void player_uppercout_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
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
      player_standing_enter(fight_context, player, enemy);
    }
    else
    {
      const int pose_idx = player->status_list[player->status_pos];
      player_set_pose(player, &player->poses[pose_idx], player_pose_left_aligned);
    }
  }
}

void player_uppercout_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_UPPERCUT;
  player->advance = player_uppercout_advance;
  player->status_pos = -1;
  player->status_list = uppercut_poses;
  player_uppercout_advance(fight_context, player, enemy);
}
