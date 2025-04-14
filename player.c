#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "player.h"
#include "log.h"
#include "sprite.h"

static const char* pose_names [] = {
  "STAND1",
  "STAND2",
  "STAND3",
  "STAND4",
  "punch1",
  "punch2",
  "punch3",
  "punch4",
  "punch5",
  "punch6",
  "walk1",
  "walk2",
  "walk3",
  "walk4",
  "walk5",
  "walk6",
  "jump1",
  "jump2",
  "jump3",
  "jump4",
  "jump5",
  "jump6",
  "jump7",
  "jump8",
  "jump9",
  "down",
  "downd",
  "downh",
  "downk1",
  "downk2",
  "downk3",
  "downk2l",
  "downp1",
  "downp2",
  "uppcut1",
  "uppcut2",
  "uppcut3",
  "uppcut4",
  "bodyp1",
  "bodyp2",
  "facep1",
  "facep2",
  "fell1",
  "fell2",
  "fell3",
  "fell4",
  "fell5",
  "fell6",
  "fell1l",
  "fell2l",
  "fell3l",
  "fell4l",
  "fell5l",
  "jumpk1",
  "jumpk2",
  "jumpp1",
  "jumpp2",
  "jumpv",
  "jumpvk",
  "kick1",
  "kick2",
  "kick3",
  "kick2l",
  "lsweep1",
  "lsweep2",
  "lsweep3",
  "lsweep4",
  "lsweep5",
  "outward1",
  "outward2",
  "outward3",
  "outward4",
  "outward5",
  "outward6",
  "punch3l",
  "punch6l",
  "defense",
  "bigp1",
  "bigp2",
  "bigp3",
  "bigp4",
  "win",
};

static bool load_pose(player_pose_t* pose, const char* player_name, const char* pose_name)
{
  bool result = false;
  char file_path[256];

  sprintf(file_path, "assets\\%s\\%s.bmp", player_name, pose_name);
  pose->sprite = create_bmp_sprite(file_path);
  if(pose->sprite != NULL)
  {
    FILE* metadata = NULL;

    result = true;

    sprintf(file_path, "assets\\%s\\%s.txt", player_name, pose_name);

    metadata = fopen(file_path, "r");
    if(metadata)
    {
      int i = 0;
      int damage_boxes = 0;
      int hit_boxes = 0;


      fscanf(metadata, "%d %d\n", &damage_boxes, &hit_boxes);
      
      for(i = 0; i < damage_boxes; ++i)
      {
        fscanf(
          metadata, 
          "%d %d %d %d\n", 
          &(pose->damage_boxes[i].x),
          &(pose->damage_boxes[i].y),
          &(pose->damage_boxes[i].sx),
          &(pose->damage_boxes[i].sy));
      }

      for(i = 0; i < hit_boxes; ++i)
      {
        fscanf(
          metadata, 
          "%d %d %d %d\n", 
          &(pose->hit_boxes[i].x),
          &(pose->hit_boxes[i].y),
          &(pose->hit_boxes[i].sx),
          &(pose->hit_boxes[i].sy));
      }
      fclose(metadata);
    }
  }
  return result;
}

player_t* player_create(const char* player_name)
{
  player_t* player = (player_t*) malloc(sizeof(player_t));
  if(player)
  {
    int i = 0;
    char file_path[256];

    memset(player, 0, sizeof(player_t));

    sprintf(file_path, "assets\\%s\\%s.bmp", player_name, "name");
    player->name = create_bmp_sprite(file_path);

    for(; i < POSE_COUNT; ++i)
    {
      if(!load_pose(&(player->poses[i]), player_name, pose_names[i]))
      {
        char error[256];

        sprintf(error, "Error loading pose %s for player %s", pose_names[i], player_name);
        log_record(error);
        break;
      }
    }

    if(player->name == NULL || i < POSE_COUNT)
    {
      player_free(player);
      player = NULL;
    }
  }
  return player;
}

void player_free(player_t* player)
{
  int i = 0;
  for(; i < POSE_COUNT; ++i)
  {
    if(player->poses[i].sprite)
    {
      sprite_free(player->poses[i].sprite);
    }
  }

  if(player->name)
  {
    sprite_free(player->name);
  }

  free(player);
}

void player_set_pose(player_t* player, player_pose_t* pose,  player_pose_alignment_t alignment)
{
  const int current_sprite_height = sprite_height(player->current_pose->sprite);  
  const int new_sprite_height =  sprite_height(pose->sprite);

  player->y_pos += (current_sprite_height - new_sprite_height);
  if((player->flipped && alignment == player_pose_left_aligned)
  || (!player->flipped && alignment == player_pose_right_aligned))
  {
    const int current_sprite_width = sprite_width(player->current_pose->sprite);
    const int current_end_x = player->x_pos + current_sprite_width;
    const int new_sprite_width = sprite_width(pose->sprite);
    player->x_pos = current_end_x - new_sprite_width;
  }
  player->current_pose = pose;
}
