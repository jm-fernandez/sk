#include "player_win.h"

static void player_win_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
}

void player_win_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_WIN;
  player->advance = player_win_advance;
  player_set_pose(player, &(player->poses[POSE_WIN]), player_pose_left_aligned);
}
