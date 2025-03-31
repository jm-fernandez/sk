#include <stddef.h>

#include "player_jump.h"
#include "player.h"
#include "player_standing.h"
#include "player_hit.h"

#include "sprite.h"

static const int y_positions_jump [] = {30, 60, 85, 105, 125, 140, 150, 155, 150, 140, 125, 105, 85, 60, 30};

static const int vertical_jump [] = {
  -40,
  -35,
  -35,
  -30,
  -25,
  -25,
  -20,
  20,
  25,
  25,
  30,
  35,
  35,
  40,
  0,
};

void player_jump_forward_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else 
  {
    player->x_pos += (player->flipped) ? -15 : 15;
    if(player->status4 >= sizeof(y_positions_jump) / sizeof(y_positions_jump[0]))
    {
      const int current_sprite_height = sprite_height(player->current_pose->sprite); 
      player->y_pos = player->status3 - current_sprite_height;
      player->flipped = (enemy->x_pos < player->x_pos);
      player_standing_enter(fight_context, player, enemy);
    }
    else
    {
      player_pose_t* new_pose = NULL;

      const int standing_height = sprite_height(player->poses[POSE_STANDING_1].sprite); 
      player->y_pos = player->status3 - y_positions_jump[player->status4++] - standing_height;

      if(player->status == STATUS_JUMP_KICK)
      {
        new_pose = &(player->poses[POSE_JUMP_KICK_2]);
      }
      else if(player->status == STATUS_JUMP_PUNCH)
      {
        new_pose = &(player->poses[POSE_JUMP_PUNCH_2]);
      }

      else if(player->status == STATUS_JUMP_FORWARD &&
        (player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK) ||
        player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))) 
      {
        player->status = STATUS_JUMP_KICK;
        new_pose = &(player->poses[POSE_JUMP_KICK_1]);
      }
      else if(player->status == STATUS_JUMP_FORWARD &&
        (player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH) ||
        player_is_key_pressed(player, PLAYER_INPUT_LOW_PUNCH))) 
      {
        player->status = STATUS_JUMP_PUNCH;
        new_pose = &(player->poses[POSE_JUMP_PUNCH_1]);
      }     
      else
      {
        if(++player->status2 > 8)
        {
          player->status2 = 1;
        }
        new_pose = &(player->poses[POSE_JUMP_1 + player->status2]);
      }
      player_set_pose(player, new_pose, player_pose_left_aligned);
    }
  }
}

void player_jump_forward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const int current_sprite_height = sprite_height(player->current_pose->sprite); 
  player->status = STATUS_JUMP_FORWARD;
  player->advance = player_jump_forward_advance;
  player->status2 = 0;
  player->status4 = 0;
  player->status3 = player->y_pos + current_sprite_height;
  player->current_pose = &(player->poses[POSE_JUMP_1]);
}

void player_jump_backward_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else 
  {
    player->x_pos += (player->flipped) ? 15 : -15;
    if(player->status4 >= sizeof(y_positions_jump) / sizeof(y_positions_jump[0]))
    {
      const int current_sprite_height = sprite_height(player->current_pose->sprite); 
      player->y_pos = player->status3 - current_sprite_height;
      player->flipped = (enemy->x_pos < player->x_pos);
      player_standing_enter(fight_context, player, enemy);
    }
    else
    {
      const int standing_height = sprite_height(player->poses[POSE_STANDING_1].sprite); 
      player->y_pos = player->status3 - y_positions_jump[player->status4++] - standing_height;

      if(player->status == STATUS_JUMP_KICK)
      {
        player->current_pose = &(player->poses[POSE_JUMP_KICK_2]);
      }
      else if(player->status == STATUS_JUMP_PUNCH)
      {
        player->current_pose = &(player->poses[POSE_JUMP_PUNCH_2]);
      }
      else if(player->status == STATUS_JUMP_BACKWARD &&
        (player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK) ||
        player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))) 
      {
        player->status = STATUS_JUMP_KICK;
        player->current_pose = &(player->poses[POSE_JUMP_KICK_1]);
      }
      else if(player->status == STATUS_JUMP_BACKWARD &&
        (player_is_key_pressed(player, PLAYER_INPUT_HIGH_PUNCH) ||
        player_is_key_pressed(player, PLAYER_INPUT_LOW_PUNCH))) 
      {
        player->status = STATUS_JUMP_PUNCH;
        player->current_pose = &(player->poses[POSE_JUMP_PUNCH_1]);
      } 
      else
      {
        if(player->status2 == 0 || player->status2 == 1)
        {
          player->status2 = 8;
        }
        --player->status2;
        player->current_pose = &(player->poses[POSE_JUMP_1 + player->status2]);    
      }
    }
  }
}

void player_jump_backward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const int current_sprite_height = sprite_height(player->current_pose->sprite); 
  player->status = STATUS_JUMP_BACKWARD;
  player->advance = player_jump_backward_advance;
  player->status2 = 0;
  player->status4 = 0;
  player->status3 = player->y_pos + current_sprite_height;
  player->current_pose = &(player->poses[POSE_JUMP_1]);
}

void player_jump_vertical_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const int jump = vertical_jump[++player->status2];
  int pose_id = (player->status2 == 0) ? POSE_JUMP_1 : POSE_JUMP_VERT;

  if(player_hit(player))
  {
    player_hit_enter(fight_context, player, enemy);
  }
  else if(player_is_key_pressed(player, PLAYER_INPUT_HIGH_KICK)
  || player_is_key_pressed(player, PLAYER_INPUT_LOW_KICK))
  {
    player->status = STATUS_JUMPV_KICK;
  }

  if(player->status == STATUS_JUMPV_KICK)
  {
    pose_id = POSE_JUMP_VERT_K;
  }
  
  if(jump)
  {
    player->y_pos += jump;
    player_set_pose(player, &player->poses[pose_id], player_pose_left_aligned);
  }
  else
  {
    player_standing_enter(fight_context, player, enemy);
  }
}

void player_jump_vertical_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_JUMP_VERTICAL;
  player->advance = player_jump_vertical_advance;
  player->status2 = -1;
  player_jump_vertical_advance(fight_context, player, enemy);
}
