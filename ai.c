#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#include "keyconfig.h"
#include "player.h"
#include "sprite.h"
#include "ai.h"

#define STOP_VALUE -2

#define AI_STATUS_WAITING               0
#define AI_STATUS_WALKING_FORWARD       1
#define AI_STATUS_WALKING_BACKWARD      2
#define AI_STATUS_DEFENSE               3
#define AI_STATUS_DOWN_DEFENSE          4

#define IN_RANGE_JUMP            320
#define IN_RANGE_DOWN_LOW_KICK   145
#define IN_RANGE_DOWN_HICH_KICK  110  // down high kick and sweep
#define IN_RANGE_KICK             60  // high and low kick, and upercout
#define IN_RANGE_PUNCH            45    

const int high_punch[] = {
    KEY_CONFIG_HIGH_PUNCH,
    KEY_CONFIG_HIGH_PUNCH_END,
    STOP_VALUE,
};

const int low_punch[] = {
    KEY_CONFIG_HIGH_PUNCH,
    KEY_CONFIG_HIGH_PUNCH_END,
    STOP_VALUE,
};

const int high_kick[] = {
    KEY_CONFIG_HIGH_KICK,
    KEY_CONFIG_HIGH_KICK_END,
    STOP_VALUE,
};

const int low_kick[] = {
    KEY_CONFIG_LOW_KICK,
    KEY_CONFIG_LOW_KICK_END,
    STOP_VALUE,
};

const int sweep[] = {
    KEY_CONFIG_LEFT,
    KEY_CONFIG_LOW_KICK,
    KEY_CONFIG_LOW_KICK_END,
    KEY_CONFIG_LEFT_END,
    STOP_VALUE,
};

const int outward[] = {
    KEY_CONFIG_LEFT,
    KEY_CONFIG_HIGH_KICK,
    KEY_CONFIG_HIGH_KICK_END,
    KEY_CONFIG_LEFT_END,
    STOP_VALUE,
};

const int jump_forward_and_kick[] = {
    KEY_CONFIG_RIGHT,
    KEY_CONFIG_UP,
    KEY_CONFIG_NONE,
    KEY_CONFIG_UP_END,
    KEY_CONFIG_RIGHT_END,
    KEY_CONFIG_NONE,
    KEY_CONFIG_NONE,
    KEY_CONFIG_HIGH_KICK,
    KEY_CONFIG_HIGH_KICK_END,
    STOP_VALUE,
};

const int jump_forward_and_punch[] = {
    KEY_CONFIG_RIGHT,
    KEY_CONFIG_UP,
    KEY_CONFIG_NONE,
    KEY_CONFIG_UP_END,
    KEY_CONFIG_RIGHT_END,
    KEY_CONFIG_NONE,
    KEY_CONFIG_NONE,
    KEY_CONFIG_HIGH_PUNCH,
    KEY_CONFIG_HIGH_PUNCH_END,
    STOP_VALUE,
};

const int jump_forward[] = {
    KEY_CONFIG_RIGHT,
    KEY_CONFIG_UP,
    KEY_CONFIG_NONE,
    KEY_CONFIG_UP_END,
    KEY_CONFIG_RIGHT_END,
    STOP_VALUE,
};

const int jump_backward[] = {
    KEY_CONFIG_LEFT,
    KEY_CONFIG_UP,
    KEY_CONFIG_NONE,
    KEY_CONFIG_UP_END,
    KEY_CONFIG_LEFT_END,
    STOP_VALUE,
};

const int down_low_kick[] = {
    KEY_CONFIG_DOWN,
    KEY_CONFIG_LOW_KICK,
    KEY_CONFIG_LOW_KICK_END,
    KEY_CONFIG_DOWN_END,
    STOP_VALUE,
};

const int down_high_kick[] = {
    KEY_CONFIG_DOWN,
    KEY_CONFIG_HIGH_KICK,
    KEY_CONFIG_HIGH_KICK_END,
    KEY_CONFIG_DOWN_END,
    STOP_VALUE,
};

const int uppercut[] = {
    KEY_CONFIG_DOWN,
    KEY_CONFIG_HIGH_PUNCH,
    KEY_CONFIG_HIGH_PUNCH_END,
    KEY_CONFIG_DOWN_END,
    STOP_VALUE,
};

static bool is_jump_attack(const player_t* player)
{
  const int enemy_status = player_status(player);
  return enemy_status == STATUS_JUMP_KICK
      || enemy_status == STATUS_JUMP_PUNCH;
}

static bool is_far_attack(const player_t* player)
{
  const int enemy_status = player_status(player);
  return enemy_status == STATUS_DOWN_LOW_KICK;
}

static bool is_med_attack(const player_t* player)
{
  const int enemy_status = player_status(player);
  return enemy_status == STATUS_DOWN_HIGH_KICK
      || enemy_status == STATUS_SWEEP;
}

static bool is_near_attack(const player_t* player)
{
  const int enemy_status = player_status(player);
  return enemy_status == STATUS_HIGH_KICK
      || enemy_status == STATUS_LOW_KICK
      || enemy_status == STATUS_UPPERCUT;
}

static bool is_close_attack(const player_t* player)
{
  const int enemy_status = player_status(player);
  return enemy_status == STATUS_HIGH_PUNCH
      || enemy_status == STATUS_LOW_PUNCH
      || enemy_status == STATUS_DOWN_PUNCH
      || enemy_status == STATUS_OUTWARD
      || enemy_status == STATUS_JUMPV_KICK;
}

static bool is_attack(const player_t* player)
{
    return is_close_attack(player)
        || is_near_attack(player)
        || is_med_attack(player)
        || is_far_attack(player)
        || is_jump_attack(player);
}

static int compute_final_key(int key, const player_t* player)
{
    const bool flipped = player_flipped(player);
    if(flipped)
    {
        switch(key)
        {
            case KEY_CONFIG_RIGHT:
                key = KEY_CONFIG_LEFT;
                break;
            case KEY_CONFIG_RIGHT_END:
                key = KEY_CONFIG_LEFT_END;
                break;
            case KEY_CONFIG_LEFT:
                key = KEY_CONFIG_RIGHT;
                break;
            case KEY_CONFIG_LEFT_END:
                key = KEY_CONFIG_RIGHT_END;
                break;
        }
    }
    return key;
}

void ai_init(ai_context_t* ai_context)
{
    int i = 0;

    srand(time(0));
    for(;i < RND_COUNT; ++i)
    {
        ai_context->rnd[i] = rand();
    }

    ai_context->rnd_index = 0;
    ai_context->status = 0;
    ai_context->counter = 0;
    ai_context->move = NULL;
}

static int distance_to_enemy(const player_t* player, const player_t* enemy)
{
    int result = 0;
    if(player->flipped)
    {

        result = player_x_pos(player) - (player_x_pos(enemy) + sprite_width(player_get_sprite(enemy)));
    }   
    else
    {
        result = player_x_pos(enemy) - (player_x_pos(player) + sprite_width(player_get_sprite(player)));
    }
    return result;
}

static int get_rnd_value(ai_context_t* ai_context)
{
    const int value = ai_context->rnd[ai_context->rnd_index];
    ai_context->rnd_index = (ai_context->rnd_index + 1) % RND_COUNT;
    return value;
}

static bool get_rnd_bool(ai_context_t* ai_context)
{
    const int value = get_rnd_value(ai_context);
    return (value & 1) != 0;
}

static int get_release_key(int key)
{
    int result = KEY_CONFIG_NONE;
    switch(key)
    {
        case KEY_CONFIG_DEFENSE:
            result = KEY_CONFIG_DEFENSE_END;
            break;
        case KEY_CONFIG_RIGHT:
            result = KEY_CONFIG_RIGHT_END;
            break;
        case KEY_CONFIG_LEFT:
            result = KEY_CONFIG_LEFT_END;
            break;
        case KEY_CONFIG_UP:
            result = KEY_CONFIG_UP_END;
            break;
        case KEY_CONFIG_DOWN:
            result = KEY_CONFIG_DOWN_END;
            break;
        case KEY_CONFIG_HIGH_KICK:
            result = KEY_CONFIG_HIGH_KICK_END;
            break;
        case KEY_CONFIG_LOW_KICK:
            result = KEY_CONFIG_LOW_KICK_END;
            break;
        case KEY_CONFIG_HIGH_PUNCH:
            result = KEY_CONFIG_HIGH_PUNCH_END;
            break;
        case KEY_CONFIG_LOW_PUNCH:
            result = KEY_CONFIG_LOW_PUNCH_END;
            break;
    }
    return result;
}

static int ai_out_or_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const bool rvalue = get_rnd_bool(ai_context);
    if(rvalue)
    {   
        ai_context->move = jump_forward + 1;
        result = compute_final_key(*jump_forward, player);
    }
    else
    {
        ai_context->status = AI_STATUS_WALKING_FORWARD;
        result = compute_final_key(KEY_CONFIG_RIGHT, player);
        ai_context->target = IN_RANGE_JUMP;
        ai_context->key = result;
    }
    return result;
}

static int ai_in_jmp_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int enemy_status = player_status(enemy);

    if(is_jump_attack(enemy))
    {
        const int value = get_rnd_value(ai_context);
        switch(value % 3)
        {
            case 0:
                ai_context->status = AI_STATUS_WALKING_BACKWARD;
                result = compute_final_key(KEY_CONFIG_LEFT, player);
                ai_context->target = IN_RANGE_DOWN_LOW_KICK;
                ai_context->key = result;
            case 1:
                ai_context->move = jump_backward;
                result = compute_final_key(*jump_backward, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                result = compute_final_key(*sweep, player);
                break;
        }
    }
    else
    {
        const int value = get_rnd_value(ai_context);
        switch(value % 5)
        {
            case 0:
                ai_context->move = jump_forward_and_kick + 1;
                result = compute_final_key(*jump_forward_and_kick, player);
                break;
            case 1:
                ai_context->move = jump_forward_and_punch + 1;
                result = compute_final_key(*jump_forward_and_punch, player);
                break;
            default:
                ai_context->status = AI_STATUS_WALKING_FORWARD;
                result = compute_final_key(KEY_CONFIG_RIGHT, player);
                ai_context->target = IN_RANGE_DOWN_LOW_KICK;
                ai_context->key = result;
                break;
        }
    }
    return result;
}

static bool is_player_in_floor(const fight_context_t* fight_context, const player_t* player)
{
    const int y0 = player_y_pos(player) + sprite_height(player_get_sprite(player));
    return y0 >= fight_context->floor;
}

static int ai_enter_walking_backward(ai_context_t* ai_context, const player_t* player)
{
    const int result = compute_final_key(KEY_CONFIG_LEFT, player);
    ai_context->status = AI_STATUS_WALKING_BACKWARD;
    ai_context->target = IN_RANGE_DOWN_LOW_KICK;
    ai_context->key = result;
    return result;
}

static int ai_enter_walking_forward(ai_context_t* ai_context, const player_t* player, int target)
{
    const int result = compute_final_key(KEY_CONFIG_RIGHT, player);
    ai_context->status = AI_STATUS_WALKING_FORWARD;
    ai_context->target = target;
    ai_context->key = result;
    return result;
}

static int ai_enter_defense(ai_context_t* ai_context, const player_t* player)
{
    const int result = compute_final_key(KEY_CONFIG_DEFENSE, player);
    ai_context->status = AI_STATUS_WALKING_BACKWARD;
    ai_context->target = IN_RANGE_DOWN_LOW_KICK;
    ai_context->key = result;
    return result;    
}

static int ai_enter_down_defense(ai_context_t* ai_context, const player_t* player)
{
    const int result = compute_final_key(KEY_CONFIG_DEFENSE, player);
    ai_context->status = AI_STATUS_DOWN_DEFENSE;
    ai_context->counter = 1;
    return result;  
}


static int ai_in_down_high_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int enemy_status = player_status(player);
    if((enemy_status == STATUS_SWEEP) || (enemy_status == STATUS_DOWN_LOW_KICK))
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            result = compute_final_key(*jump_backward, player);
        }
        else
        {
            result = ai_enter_walking_backward(ai_context, player);
        }
    }
    else if(enemy_status == STATUS_DOWN_HIGH_KICK)
    {
        if(get_rnd_bool(ai_context))
        {
            result = ai_enter_walking_backward(ai_context, player);
        }
        else
        {
            result = ai_enter_down_defense(ai_context, player);
        }
    }
    else if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
            result = ai_enter_down_defense(ai_context, player);
    }
    else
    {
        switch(get_rnd_value(ai_context) % 3)
        {
            case 0:
                ai_context->move = down_low_kick + 1;
                result = compute_final_key(*down_low_kick, player);
                break;
            case 1:
                ai_context->move = down_high_kick + 1;
                result = compute_final_key(*down_high_kick, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                result = compute_final_key(*sweep, player);
                break;
        }
    }
    return result;
}

static int ai_in_down_low_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    if(is_far_attack(enemy))
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            result = compute_final_key(*jump_backward, player);
        }
        else
        {
            result = ai_enter_walking_backward(ai_context, player);
        }
    }
    else if(is_jump_attack(enemy))
    {
        if(get_rnd_bool(ai_context))
        {
            result = ai_enter_defense(ai_context, player);
        }
        else
        {
            result = ai_enter_walking_forward(ai_context, player, IN_RANGE_DOWN_HICH_KICK);
        }
    }
    else if(is_player_in_floor(fight_context, enemy))
    {
        ai_context->move = down_low_kick + 1;
        result = compute_final_key(*down_low_kick, player);
    }
    else
    {
        int enemy_status = player_status(enemy);
        if(enemy_status == STATUS_JUMP_FORWARD)
        {
            switch(get_rnd_value(ai_context) % 3)
            {
                case 0:
                    ai_context->move = high_kick + 1;
                    result = compute_final_key(*high_kick, player);
                case 1:
                    ai_context->move = high_punch + 1;
                    result = compute_final_key(*high_punch, player);
                default:
                    ai_context->move = outward + 1;
                    result = compute_final_key(*outward, player);
            }
        }
    }
    return result;
}

static int ai_in_punch_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int enemy_status = player_status(player);
    if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
        result = ai_enter_defense(ai_context, player);
    }
    else if(enemy_status == STATUS_SWEEP || enemy_status == STATUS_DOWN_LOW_KICK)
    {
        ai_context->move = jump_backward + 1;
        result = compute_final_key(*jump_backward, player);
    }
    else if(enemy_status == STATUS_UPPERCUT || enemy_status == STATUS_LOW_KICK)
    {
        result = ai_enter_defense(ai_context, player);
    }
    else if(enemy_status == STATUS_HIGH_KICK || enemy_status == STATUS_HIGH_PUNCH)
    {
        switch(get_rnd_value(ai_context) % 4)
        {
            case 0:
                result = ai_enter_defense(ai_context, player);
                break;
            case 1:
                ai_context->move = down_low_kick + 1;
                result = compute_final_key(*down_low_kick, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                result = compute_final_key(*sweep, player);
                break;
            case 3:
                result = ai_enter_down_defense(ai_context, player);
                break;
        }
    }
    else
    {
        switch(get_rnd_value(ai_context) % 5)
        {
            case 0:
                ai_context->move = low_kick + 1;
                result = compute_final_key(*low_kick, player);            
            case 1:
                ai_context->move = high_kick + 1;
                result = compute_final_key(*high_kick, player);
            case 2:
                ai_context->move = uppercut + 1;
                result = compute_final_key(*uppercut, player);
            case 3:
                ai_context->move = high_punch + 1;
                result = compute_final_key(*high_punch, player);
            case 4:
                ai_context->move = low_punch + 1;
                result = compute_final_key(*low_punch, player);
        }
    }
    return result;
}

static int ai_in_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int enemy_status = player_status(player);
    if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
        if(get_rnd_bool(ai_context))
        {
            result = ai_enter_defense(ai_context, player);
        }
        else
        {
            ai_context->move = uppercut + 1;
            result = compute_final_key(*uppercut, player);            
        }
    }
    else if(enemy_status == STATUS_SWEEP || enemy_status == STATUS_DOWN_LOW_KICK)
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            result = compute_final_key(*jump_backward, player);
        }
        else
        {
            result = ai_enter_walking_backward(ai_context, player);
        }
    }
    else if(enemy_status == STATUS_UPPERCUT || enemy_status == STATUS_LOW_KICK)
    {
        result = ai_enter_defense(ai_context, player);
    }
    else if(enemy_status == STATUS_HIGH_KICK || enemy_status == STATUS_HIGH_PUNCH)
    {
        switch(get_rnd_value(ai_context) % 4)
        {
            case 0:
                result = ai_enter_defense(ai_context, player);
                break;
            case 1:
                ai_context->move = down_low_kick + 1;
                result = compute_final_key(*down_low_kick, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                result = compute_final_key(*sweep, player);
                break;
            case 3:
                result = ai_enter_down_defense(ai_context, player);
                break;
        }
    }
    else
    {
        switch(get_rnd_value(ai_context) % 2)
        {
            case 0:
                ai_context->move = low_kick + 1;
                result = compute_final_key(*low_kick, player);            
            case 1:
                ai_context->move = high_kick + 1;
                result = compute_final_key(*high_kick, player);
        }
    }
    return result;
}

static int ai_waiting(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int distance = distance_to_enemy(player, enemy);
    const int current_status = player_status(player);

    if(current_status == STATUS_STANDING)
    {
        if(distance > IN_RANGE_JUMP)
        {
            result = ai_out_or_range(ai_context, fight_context, player, enemy);
        }
        else if(distance < IN_RANGE_JUMP && distance > IN_RANGE_DOWN_LOW_KICK)
        {
            result = ai_in_jmp_range(ai_context, fight_context, player, enemy);
        }
        else if(distance < IN_RANGE_DOWN_LOW_KICK && distance > IN_RANGE_DOWN_HICH_KICK)
        {
            result = ai_in_down_low_kick_range(ai_context, fight_context, player, enemy);
        }
        else if(distance < IN_RANGE_DOWN_HICH_KICK && distance > IN_RANGE_KICK)
        {
            result = ai_in_down_high_kick_range(ai_context, fight_context, player, enemy);
        }
        else if((distance < IN_RANGE_KICK) && (distance > IN_RANGE_PUNCH))
        {
             result = ai_in_kick_range(ai_context, fight_context, player, enemy);
        }
        else if(distance < IN_RANGE_PUNCH)
        {
             result = ai_in_punch_range(ai_context, fight_context, player, enemy);
        }

        // else if(distance > 200)
        // {
        //     if(enemy_status == STATUS_JUMP_FORWARD
        //     || enemy_status == STATUS_JUMP_KICK
        //     || enemy_status == STATUS_JUMP_PUNCH)
        //     {
        //         const bool rvalue = get_rnd_bool(ai_context);
        //         if(rvalue)
        //         {
        //             ai_context->move = down_low_kick + 1;
        //             result = compute_final_key(*down_low_kick, player);                        
        //         }
        //         else {
        //             const bool rvalue2 = get_rnd_bool(ai_context);
        //             if(rvalue2)
        //             {
        //                 ai_context->status = AI_STATUS_DEFENSE;
        //                 result = compute_final_key(KEY_CONFIG_DEFENSE, player);
        //                 ai_context->key = result;
        //             }
        //             else
        //             {
        //                 ai_context->move = jump_backward + 1;
        //                 result = compute_final_key(*jump_backward, player);                              
        //             }
        //         }
        //     }
        //     else
        //     {
        //         const bool rvalue = get_rnd_bool(ai_context);
        //         if(rvalue)
        //         {
        //             ai_context->move = jump_forward_and_kick + 1;
        //             result = compute_final_key(*jump_forward_and_kick, player);
        //         }
        //         else
        //         {
        //             ai_context->move = down_low_kick + 1;
        //             result = compute_final_key(*down_low_kick, player);
        //         }
        //     }
        // }
        // else
        // {
        //     const bool rvalue = get_rnd_bool(ai_context);
        //     if(rvalue)
        //     {
        //         ai_context->move = jump_forward_and_kick + 1;
        //         result = compute_final_key(*jump_forward_and_kick, player);
        //     }
        //     else
        //     {
        //         ai_context->move = down_low_kick + 1;
        //         result = compute_final_key(*down_low_kick, player);
        //     }
        // }
    }
    return result;
}

static bool is_attacking_and_in_range(const player_t* player, const player_t* enemy)
{
    const int distance = distance_to_enemy(player, enemy);
    return (is_jump_attack(enemy) && (distance < IN_RANGE_JUMP))
        || (is_far_attack(enemy) && (distance < IN_RANGE_DOWN_LOW_KICK))
        || (is_med_attack(enemy) && (distance < IN_RANGE_DOWN_HICH_KICK))
        || (is_near_attack(enemy) && (distance < IN_RANGE_KICK))
        || (is_close_attack(enemy) && (distance < IN_RANGE_PUNCH));
}

static int ai_walking_forward(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int distance = distance_to_enemy(player, enemy);
    const int enemy_status = player_status(enemy);

    if((distance < ai_context->target)
    || (is_attacking_and_in_range(player, enemy)))
    {
        ai_context->status = AI_STATUS_WAITING;
        result = get_release_key(ai_context->key);
    }
    return result;
}

static int ai_defense(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    if(!is_attacking_and_in_range(player, enemy))
    {
        ai_context->status = AI_STATUS_WAITING;
        result = KEY_CONFIG_DEFENSE_END;
    }
    return result;
}

static int ai_down_defense(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    if(ai_context->counter == 1)
    {
        ai_context->status = 2;
        result = KEY_CONFIG_DOWN;
    }
    else if(ai_context->counter == 2)
    {
        if(!is_attacking_and_in_range(player, enemy))
        {
            ai_context->status = 4;
            result = KEY_CONFIG_DOWN_END;
        }
    }
    else
    {
        ai_context->status = AI_STATUS_WAITING;
        result = KEY_CONFIG_DEFENSE_END;
    }
    return result;
}

static int ai_walking_backward(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    const int distance = distance_to_enemy(player, enemy);
    if(distance < IN_RANGE_PUNCH || distance > IN_RANGE_JUMP)
    {
        ai_context->status = AI_STATUS_WAITING;
        result = get_release_key(ai_context->key);
    }
    return result;
}

int ai_last_key(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    int result = KEY_CONFIG_NONE;
    if(ai_context->move == NULL || *ai_context->move == STOP_VALUE)
    {
        switch(ai_context->status)
        {
            case AI_STATUS_WAITING:
                result = ai_waiting(ai_context, fight_context, player, enemy);
                break;
            case AI_STATUS_WALKING_FORWARD:
                result = ai_walking_forward(ai_context, fight_context, player, enemy);
                break;
            case AI_STATUS_WALKING_BACKWARD:
                result = ai_walking_backward(ai_context, fight_context, player, enemy);
                break;
            case AI_STATUS_DEFENSE:
                result = ai_defense(ai_context, fight_context, player, enemy);
                break;
            case AI_STATUS_DOWN_DEFENSE:
                result = ai_down_defense(ai_context, fight_context, player, enemy);
        }
    }
    else {
        result = compute_final_key(*ai_context->move++, player);
    }
    return result;
}
