#include "player_defense.h"
#include "player.h"
#include "player_hit.h"
#include "player_standing.h"
#include "player_down.h"

static void player_defense_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->flipped = (enemy->x_pos < player->x_pos);  
  
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(!player_is_key_pressed(player, PLAYER_INPUT_DEFENSE))
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
    if(player_is_key_pressed(player, PLAYER_INPUT_DOWN))
    {
      player->status = STATUS_DOWN_DEFENSE;
      player_set_pose(player, &(player->poses[POSE_DOWN_DEF]), player_pose_left_aligned);
    }
    else
    {
      player->status = STATUS_DEFENSE;
      player_set_pose(player, &(player->poses[POSE_DEFENSE]), player_pose_left_aligned);
    } 
  }
}

void player_defense_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = player_is_key_pressed(player, PLAYER_INPUT_DOWN) ? STATUS_DOWN_DEFENSE : STATUS_DEFENSE;
  player->advance = player_defense_advance;
  player_defense_advance(fight_context, player, enemy);
}
