#ifndef PLAYER_H__
#define PLAYER_H__

#include "sprite.h"
#include "stdbool.h"
#include "util.h"
#include "player_input.h"

#define POSE_STANDING_1      0
#define POSE_STANDING_2      1
#define POSE_STANDING_3      2
#define POSE_STANDING_4      3
#define POSE_PUNCH_1         4
#define POSE_PUNCH_2         5
#define POSE_PUNCH_3         6
#define POSE_PUNCH_4         7
#define POSE_PUNCH_5         8
#define POSE_PUNCH_6         9
#define POSE_WALK_1         10
#define POSE_WALK_2         11
#define POSE_WALK_3         12
#define POSE_WALK_4         13
#define POSE_WALK_5         14
#define POSE_WALK_6         15
#define POSE_JUMP_1         16
#define POSE_JUMP_2         17
#define POSE_JUMP_3         18
#define POSE_JUMP_4         19
#define POSE_JUMP_5         20
#define POSE_JUMP_6         21
#define POSE_JUMP_7         22
#define POSE_JUMP_8         23
#define POSE_JUMP_9         24
#define POSE_DOWN           25
#define POSE_DOWN_DEF       26
#define POSE_DOWN_HIT       27
#define POSE_DOWN_KICK_1    28
#define POSE_DOWN_KICK_2    29
#define POSE_DOWN_KICK_3    30
#define POSE_DOWN_KICK_L_2  31
#define POSE_DOWN_PUNCH_1   32
#define POSE_DOWN_PUNCH_2   33
#define POSE_UPPERCUT_1     34
#define POSE_UPPERCUT_2     35
#define POSE_UPPERCUT_3     36
#define POSE_UPPERCUT_4     37
#define POSE_BODYP_1        38
#define POSE_BODYP_2        39
#define POSE_FACEP_1        40
#define POSE_FACEP_2        41
#define POSE_FELL_1         42
#define POSE_FELL_2         43
#define POSE_FELL_3         44
#define POSE_FELL_4         45
#define POSE_FELL_5         46
#define POSE_FELL_6         47
#define POSE_FELL_1_L       48
#define POSE_FELL_2_L       49
#define POSE_FELL_3_L       50
#define POSE_FELL_4_L       51
#define POSE_FELL_5_L       52
#define POSE_JUMP_KICK_1    53
#define POSE_JUMP_KICK_2    54
#define POSE_JUMP_PUNCH_1   55
#define POSE_JUMP_PUNCH_2   56
#define POSE_JUMP_VERT      57
#define POSE_JUMP_VERT_K    58
#define POSE_KICK_1         59
#define POSE_KICK_2         60
#define POSE_KICK_3         61
#define POSE_KICK_2_L       62
#define POSE_LSWEEP_1       63
#define POSE_LSWEEP_2       64
#define POSE_LSWEEP_3       65
#define POSE_LSWEEP_4       66
#define POSE_LSWEEP_5       67
#define POSE_OUTWARD_1      68
#define POSE_OUTWARD_2      69
#define POSE_OUTWARD_3      70
#define POSE_OUTWARD_4      71
#define POSE_OUTWARD_5      72
#define POSE_OUTWARD_6      73
#define POSE_PUNCH_3L       74
#define POSE_PUNCH_6L       75
#define POSE_DEFENSE        76
#define POSE_BIGP_1         77
#define POSE_BIGP_2         78
#define POSE_BIGP_3         79
#define POSE_BIGP_4         80
#define POSE_WIN            81
#define POSE_COUNT          82

#define STATUS_STANDING         0
#define STATUS_HIGH_PUNCH       1
#define STATUS_LOW_PUNCH        2
#define STATUS_WALK_FORWARD     3
#define STATUS_WALK_BACKWARD    4
#define STATUS_JUMP_FORWARD     5
#define STATUS_JUMP_BACKWARD    6
#define STATUS_JUMP_VERTICAL    7
#define STATUS_DOWN             8
#define STATUS_UPPERCUT         9
#define STATUS_HIT             10
#define STATUS_DEFENSE         11
#define STATUS_LOW_KICK        12
#define STATUS_HIGH_KICK       13
#define STATUS_OUTWARD         14
#define STATUS_DOWN_LOW_KICK   15
#define STATUS_DOWN_HIGH_KICK  16
#define STATUS_DOWN_PUNCH      17
#define STATUS_DOWN_HIT        18
#define STATUS_DOWN_DEFENSE    19
#define STATUS_SWEEP           20
#define STATUS_JUMP_KICK       21
#define STATUS_JUMP_PUNCH      22
#define STATUS_FALL_1          23
#define STATUS_FALL_2          24
#define STATUS_FALL_3          25
#define STATUS_FALL_4          26
#define STATUS_STAND_UP        27
#define STATUS_EQ              28
#define STATUS_JUMPV_KICK      29
#define STATUS_KO              30
#define STATUS_WIN             31
#define STATUS_COUNT           32

#define PLAYER_POSE_MAX_BOXES           4

typedef struct fight_context_t_
{
    bool fighting;
    int width;
    int floor;
    int remaining_time;
}fight_context_t;

typedef enum player_pose_alignment_t_
{
    player_pose_left_aligned,
    player_pose_right_aligned,
}player_pose_alignment_t;

struct player_t_;
typedef void (*player_advance_t)(const fight_context_t* fight_context, struct player_t_* player, struct player_t_* enemy);

typedef struct player_pose_t_
{
    sprite_t* sprite;
    square_t damage_boxes[PLAYER_POSE_MAX_BOXES];
    square_t hit_boxes[PLAYER_POSE_MAX_BOXES];
}player_pose_t;


typedef struct player_t_
{
  int x_pos;
  int y_pos;
  bool flipped;
  player_input_t* input;
  player_advance_t advance;
  player_pose_t poses[POSE_COUNT];
  player_pose_t* current_pose;
  sprite_t* name;
  bool hit;
  int x_hit;
  int y_hit;
  int hit_status;
  int life;
  int wins;
  int status;
  int status2;
  int status3;
  int status4;
  int status_pos;
  const int* status_list;
} player_t;

void player_free(player_t* player);
player_t* player_create(const char* player_name);

void player_set_pose(player_t* player, player_pose_t* pose, player_pose_alignment_t alignment);

static void player_set_input(player_t* player, player_input_t* input)
{
    player->input = input;
}

static bool player_is_key_pressed(const player_t* player, int key)
{
    bool result = false;
    if(player->input)
    {
        result = player_input_is_key_pressed(player->input, key);
    }
    return result;
}

static sprite_t* player_get_sprite(const player_t* player)
{
    return player->current_pose->sprite;
}

static int player_x_pos(const player_t* player)
{
    return player->x_pos;
}

static void player_set_x_pos(player_t* player, int x_pos)
{
    player->x_pos = x_pos;
}

static int player_y_pos(const player_t* player)
{
    return player->y_pos;
}

static bool player_flipped(const player_t* player)
{
    return player->flipped;
}

static void player_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
    player->advance(fight_context, player, enemy);
}

static player_pose_t* player_get_pose(const player_t* player)
{
    return player->current_pose;
}

static void player_set_hit(player_t* player, int x_hit, int y_hit, int hit_status)
{
    player->hit = true;
    player->x_hit = x_hit;
    player->y_hit = y_hit;
    player->hit_status = hit_status;
}

static void player_clear_hit(player_t* player)
{
    player->hit = false;
}

static bool player_hit(const player_t* player)
{
    return player->hit;
}

static int player_status(const player_t* player)
{
    return player->status;
}

static int player_life(const player_t* player)
{
    return player->life;
}

static void player_set_life(player_t* player, int life)
{
    player->life = life;
}

static sprite_t* player_name(const player_t* player)
{
    return player->name;
}

static int player_wins(const player_t* player)
{
    return player->wins;
}

static void player_set_wins(player_t* player, int wins)
{
    player->wins = wins;
}

#endif // PLAYER_H__
