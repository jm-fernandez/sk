#include "player_down.h"
#include "player.h"
#include "player_standing.h"
#include "player_uppercout.h"
#include "player_defense.h"
#include "player_kick.h"
#include "player_punch.h"
#include "player_hit.h"

static void player_down_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->flipped = (enemy->x_pos < player->x_pos);  

  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_DEFENSE))
  {
    player_defense_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))
  {
    player_down_low_kick_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK))
  {
    player_down_high_kick_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH))
  {
      player_uppercout_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_LOW_PUNCH))
  {
      player_down_punch_enter(fight_context, player, enemy);
  }
  else if(!player_is_key_pressed(player, PLAYER_INPUT_DOWN))
  {
      player_standing_enter(fight_context, player, enemy);
  }
}

void player_down_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_DOWN;
  player->advance = player_down_advance;
  player_set_pose(player, &(player->poses[POSE_DOWN]), player_pose_left_aligned);
}
