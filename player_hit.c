#include "player.h"
#include "player_hit.h"
#include "player_standing.h"
#include "player_down.h"
#include "player_ko.h"
#include "sprite.h"

#define HUGE_LIFE_HIT           20
#define HIGH_LIFE_HIT           15
#define MED_LIFE_HIT            15
#define LOW_LIFE_HIT            5

#define FACE_HIT_FIRST_POSE POSE_FACEP_1
#define BODY_HIT_FIRST_POSE POSE_BODYP_1
#define HIT_POSE_COUNT 2

static int face_punch_poses[] = {
  POSE_FACEP_1,
  POSE_FACEP_2,
  POSE_FACEP_2,
  POSE_FACEP_1,
  -1,
};

static int body_punch_poses[] = {
  POSE_BODYP_1,
  POSE_BODYP_2,
  POSE_BODYP_2,
  POSE_BODYP_1,
  -1,
};

static int eq_poses [] = {
  POSE_BIGP_1,
  POSE_BIGP_1,
  POSE_BIGP_2,
  POSE_BIGP_2,
  POSE_BIGP_3,
  POSE_BIGP_3,
  POSE_BIGP_4,
  POSE_BIGP_4,
  -1,
};

static int fell_poses [] = {
  POSE_FELL_1,
  POSE_FELL_1,
  POSE_FELL_2,
  POSE_FELL_2,
  POSE_FELL_3,
  POSE_FELL_3,
  POSE_FELL_4,
  POSE_FELL_4,
  POSE_FELL_5,
  POSE_FELL_5,
  POSE_FELL_5,
  POSE_FELL_5,
  -1,
};

static int fell_poses_2[] = {
  POSE_FELL_1_L,
  POSE_FELL_1_L,
  POSE_FELL_1_L,
  POSE_FELL_2_L,
  POSE_FELL_2_L,
  POSE_FELL_2_L,
  POSE_FELL_3_L,
  POSE_FELL_3_L,
  POSE_FELL_3_L,
  POSE_FELL_4_L,
  POSE_FELL_4_L,
  POSE_FELL_4_L,
  POSE_FELL_5_L,
  POSE_FELL_5_L,
  POSE_FELL_5_L,
  POSE_FELL_5_L,
  POSE_FELL_5_L,
  POSE_FELL_5_L,
  -1,
};

static int fell_heights_2[] = {
  -40,
  -30,
  -30,
  -25,
  -25,
  -25,
  -15,
  15,
  25,
  25,
  25,
  30,
  30,
  40,
  0,
  0,
  0,
  0,
};

static int fell_heights[] = {
  -10,
  -10,
  -10,
  -10,
  -10,
  10,
  10,
  10,
  10,
  10,
  0,
  0,
  0,
};

static int down_hit_poses[] = {
  POSE_DOWN_HIT,
  POSE_DOWN_HIT,
  -1,
};

static int stand_up_poses [] = {
    POSE_FELL_6,
    POSE_FELL_6,
    POSE_FELL_6,
    POSE_FELL_6,
    -1,
};

static void player_hit_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{

  if((player->status == STATUS_FALL_1)
       || (player->status == STATUS_FALL_2)
       || (player->status == STATUS_FALL_3)
       || (player->status == STATUS_FALL_4))
  {
    ++player->status_pos;
    if(player->status_list[player->status_pos] == -1)
    {
      player->y_pos = fight_context->floor - sprite_height(player_get_sprite(player));
      if(player->life == 0)
      {
        player_ko_enter(fight_context, player, enemy);
      }
      else
      {
        player->status = STATUS_STAND_UP;
        player->status_pos = -1;
        player->status_list = stand_up_poses;
      }
    }
    else
    {
      const int pose_idx = player->status_list[player->status_pos];
      if(player->status == STATUS_FALL_1)
      {
        player->y_pos += fell_heights[player->status_pos];
      }
      else if (player->status == STATUS_FALL_2)
      {
        const int y_delta = fell_heights_2[player->status_pos];
        player->y_pos += y_delta;
        if(y_delta != 0)
        {
          player->x_pos += player->flipped ? 10 : -10;
        }
      }
      else if(player->status == STATUS_FALL_3)
      {
        const int pose_height = sprite_height(player->poses[pose_idx].sprite);
        const int floor_y_pos = fight_context->floor - pose_height;
        player->y_pos = min(floor_y_pos, player->y_pos + 15);
        player->x_pos += player->flipped ? 10 : -10;
        if(player->y_pos != floor_y_pos && player->status_list[player->status_pos + 1] == -1)
        {
          --player->status_pos;
        }
      }
      else if (player->status == STATUS_FALL_4)
      {
        if(fell_heights_2[player->status_pos] != 0)
        {
          player->x_pos += player->flipped ? 2 : -2;
        }
      }
      player_set_pose(player, &player->poses[pose_idx], player_pose_right_aligned);
    }
  }
  else if(player->status == STATUS_EQ)
  {
    ++player->status_pos;
    if(player->status_list[player->status_pos] == -1)
    {
      player->hit = false;
      player_standing_enter(fight_context, player, enemy);
    }
    else
    {
      const int pose_idx = player->status_list[player->status_pos];
      player->x_pos += player->flipped ? 15 : -15;
      player_set_pose(player, &player->poses[pose_idx], player_pose_left_aligned);
    }
  }
  else
  {
    ++player->status_pos;
    if(player->status_list[player->status_pos] == -1)
    {
        player->hit = false;
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

void player_hit_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  const int y_position =  player_y_pos(player) + sprite_height(player_get_sprite(player));
  if(y_position < fight_context->floor)
  {
    int non_zero_entries = 0;
    while(fell_heights_2[non_zero_entries] != 0)
    {
      ++non_zero_entries;
    }

    player->life = max(0, player->life - MED_LIFE_HIT);
    player->status = STATUS_FALL_3;
    player->status_pos = -1;
    player->status_list = fell_poses_2;
  }
  else if(enemy->status == STATUS_JUMP_PUNCH
        || enemy->status == STATUS_HIGH_KICK
        || enemy->status == STATUS_HIT)
  {
    player->life = max(0, player->life - MED_LIFE_HIT);
    if(player->life == 0)
    {
      player->status = STATUS_FALL_4;
      player->status_pos = -1;
      player->status_list = fell_poses_2;
    }
    else
    {
      player->status = STATUS_EQ;
      player->status_pos = -1;
      player->status_list = eq_poses;
    }
  }
  else if(enemy->status == STATUS_HIGH_PUNCH)
  {
    player->life = max(0, player->life - LOW_LIFE_HIT);
    if(player->life == 0)
    {
      player->life = max(0, player->life - HUGE_LIFE_HIT);
      player->status = STATUS_FALL_4;
      player->status_pos = -1;
      player->status_list = fell_poses_2;
    }
    else
    {
      player->status = STATUS_HIT;
      player->status_pos = -1;
      player->status_list = face_punch_poses;
    }
  }
  else if(enemy->status == STATUS_UPPERCUT 
    || enemy->status == STATUS_OUTWARD
    || enemy->status == STATUS_JUMP_KICK)
  {
    player->life = max(0, player->life - HUGE_LIFE_HIT);
    player->status = STATUS_FALL_2;
    player->status_pos = -1;
    player->status_list = fell_poses_2;
  }
  else if(enemy->status == STATUS_SWEEP)
  {
    player->life = max(0, player->life - HIGH_LIFE_HIT);
    player->status = STATUS_FALL_1;
    player->status_pos = -1;
    player->status_list = fell_poses;
  }
  else if(player->status == STATUS_DOWN
       || player->status == STATUS_DOWN_HIGH_KICK
       || player->status == STATUS_DOWN_LOW_KICK
       || player->status == STATUS_DOWN_PUNCH
       || player->status == STATUS_DOWN_DEFENSE)
  {
    player->life = max(0, player->life - LOW_LIFE_HIT);
    if(player->life == 0)
    {
      player->status = STATUS_FALL_4;
      player->status_pos = -1;
      player->status_list = fell_poses_2;
    }
    else
    {
      player->status = STATUS_DOWN_HIT;
      player->status_pos = -1;
      player->status_list = down_hit_poses;    
    }
  }
  else
  {
    player->life = max(0, player->life - LOW_LIFE_HIT);
    if(player->life == 0)
    {
      player->status = STATUS_FALL_4;
      player->status_pos = -1;
      player->status_list = fell_poses_2;
    }
    else
    {
      player->status = STATUS_HIT;
      player->status_pos = -1;
      player->status_list = body_punch_poses;
    }
  }

  player->advance = player_hit_advance;
  player_hit_advance(fight_context, player, enemy);
}
