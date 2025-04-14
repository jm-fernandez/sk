#include <time.h>

#include "sw_player_input.h"
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

const ai_key_event_t high_punch[] = {
    {PLAYER_INPUT_HIGH_PUNCH, true},
    {PLAYER_INPUT_HIGH_PUNCH, false},
    {STOP_VALUE, false},
};

const ai_key_event_t low_punch[] = {
    {PLAYER_INPUT_LOW_PUNCH, true},
    {PLAYER_INPUT_LOW_PUNCH, false},
    {STOP_VALUE, false},
};

const ai_key_event_t high_kick[] = {
    {PLAYER_INPUT_HIGH_KICK, true},
    {PLAYER_INPUT_HIGH_KICK, false},
    {STOP_VALUE, false},
};

const ai_key_event_t low_kick[] = {
    {PLAYER_INPUT_LOW_KICK, true},
    {PLAYER_INPUT_LOW_KICK, false},
    {STOP_VALUE, false},
};

const ai_key_event_t sweep[] = {
    {PLAYER_INPUT_LEFT, true},
    {PLAYER_INPUT_LOW_KICK, true},
    {PLAYER_INPUT_LOW_KICK, false},
    {PLAYER_INPUT_LEFT, false},
    {STOP_VALUE, false},
};

const ai_key_event_t outward[] = {
    {PLAYER_INPUT_LEFT, true},
    {PLAYER_INPUT_HIGH_KICK, true},
    {PLAYER_INPUT_HIGH_KICK, false},
    {PLAYER_INPUT_LEFT, false},
    {STOP_VALUE, false},
};

const ai_key_event_t jump_forward_and_kick[] = {
    {PLAYER_INPUT_RIGHT, true},
    {PLAYER_INPUT_UP, true},
    {PLAYER_INPUT_UP, false},
    {PLAYER_INPUT_RIGHT, false},
    {PLAYER_INPUT_HIGH_KICK, true},
    {PLAYER_INPUT_HIGH_KICK, false},
    {STOP_VALUE, false},
};

const ai_key_event_t jump_forward_and_punch[] = {
    {PLAYER_INPUT_RIGHT, true},
    {PLAYER_INPUT_UP, true},
    {PLAYER_INPUT_UP, false},
    {PLAYER_INPUT_RIGHT, false},
    {PLAYER_INPUT_HIGH_PUNCH, true},
    {PLAYER_INPUT_HIGH_PUNCH, false},
    {STOP_VALUE, false},
};

const ai_key_event_t jump_forward[] = {
    {PLAYER_INPUT_RIGHT, true},
    {PLAYER_INPUT_UP, true},
    {PLAYER_INPUT_UP, false},
    {PLAYER_INPUT_RIGHT, false},
    {STOP_VALUE, false},
};

const ai_key_event_t jump_backward[] = {
    {PLAYER_INPUT_LEFT, true},
    {PLAYER_INPUT_UP, true},
    {PLAYER_INPUT_UP, false},
    {PLAYER_INPUT_LEFT, false},
    {STOP_VALUE, false},
};

const ai_key_event_t down_low_kick[] = {
    {PLAYER_INPUT_DOWN, true},
    {PLAYER_INPUT_LOW_KICK, true},
    {PLAYER_INPUT_LOW_KICK, false},
    {PLAYER_INPUT_DOWN, false},
    {STOP_VALUE, false},
};

const ai_key_event_t down_high_kick[] = {
    {PLAYER_INPUT_DOWN, true},
    {PLAYER_INPUT_HIGH_KICK, true},
    {PLAYER_INPUT_HIGH_KICK, false},
    {PLAYER_INPUT_DOWN, false},
    {STOP_VALUE, false},
};

const ai_key_event_t uppercut[] = {
    {PLAYER_INPUT_DOWN, true},
    {PLAYER_INPUT_HIGH_PUNCH, true},
    {PLAYER_INPUT_HIGH_PUNCH, false},
    {PLAYER_INPUT_DOWN, false},
    {STOP_VALUE, false},
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
            case PLAYER_INPUT_RIGHT:
                key = PLAYER_INPUT_LEFT;
                break;
            case PLAYER_INPUT_LEFT:
                key = PLAYER_INPUT_RIGHT;
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

static void ai_out_or_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const bool rvalue = get_rnd_bool(ai_context);
    if(rvalue)
    {   
        ai_context->move = jump_forward + 1;
        *event = *jump_forward;
        event->key = compute_final_key( event->key, player);
    }
    else
    {
        ai_context->status = AI_STATUS_WALKING_FORWARD;
        event->key = compute_final_key(PLAYER_INPUT_RIGHT, player);
        event->status = true;
        ai_context->target = IN_RANGE_JUMP;
        ai_context->key = event->key;
    }
}

static void ai_in_jmp_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int enemy_status = player_status(enemy);
    if(is_jump_attack(enemy))
    {
        const int value = get_rnd_value(ai_context);
        switch(value % 3)
        {
            case 0:
                ai_context->status = AI_STATUS_WALKING_BACKWARD;
                event->key = compute_final_key(PLAYER_INPUT_LEFT, player);
                event->status = true;
                ai_context->target = IN_RANGE_DOWN_LOW_KICK;
                ai_context->key = event->status;
            case 1:
                ai_context->move = jump_backward + 1;
                *event = *jump_backward;
                event->key = compute_final_key(event->key, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                *event = *sweep;
                event->key = compute_final_key(event->key, player);
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
                *event = *jump_forward_and_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 1:
                ai_context->move = jump_forward_and_punch + 1;
                *event = *jump_forward_and_punch;
                event->key = compute_final_key(event->key, player);
                break;
            default:
                ai_context->status = AI_STATUS_WALKING_FORWARD;
                event->key = compute_final_key( PLAYER_INPUT_RIGHT, player);
                event->status = true;
                ai_context->target = IN_RANGE_DOWN_LOW_KICK;
                ai_context->key = event->key;
                break;
        }
    }
}

static bool is_player_in_floor(const fight_context_t* fight_context, const player_t* player)
{
    const int y0 = player_y_pos(player) + sprite_height(player_get_sprite(player));
    return y0 >= fight_context->floor;
}

static void ai_enter_walking_backward(ai_context_t* ai_context, const player_t* player, ai_key_event_t* event)
{
    event->key = compute_final_key(PLAYER_INPUT_LEFT, player);
    event->status = true;

    ai_context->status = AI_STATUS_WALKING_BACKWARD;
    ai_context->target = IN_RANGE_DOWN_LOW_KICK;
    ai_context->key = event->key;
}

static void ai_enter_walking_forward(ai_context_t* ai_context, const player_t* player, ai_key_event_t* event, int target)
{
    event->key = compute_final_key(PLAYER_INPUT_RIGHT, player);
    event->status = true;
    ai_context->status = AI_STATUS_WALKING_FORWARD;
    ai_context->target = target;
    ai_context->key = event->key;
}

static void ai_enter_defense(ai_context_t* ai_context, const player_t* player, ai_key_event_t* event)
{
    event->key = compute_final_key(PLAYER_INPUT_DEFENSE, player);
    event->status = true;

    ai_context->status = AI_STATUS_WALKING_BACKWARD;
    ai_context->target = IN_RANGE_DOWN_LOW_KICK;
    ai_context->key = event->key;
}

static void ai_enter_down_defense(ai_context_t* ai_context, const player_t* player, ai_key_event_t* event)
{
    event->key = compute_final_key(PLAYER_INPUT_DEFENSE, player);
    event->status = true;
    ai_context->status = AI_STATUS_DOWN_DEFENSE;
    ai_context->counter = 1;
}


static void ai_in_down_high_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int enemy_status = player_status(player);
    if((enemy_status == STATUS_SWEEP) || (enemy_status == STATUS_DOWN_LOW_KICK))
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            *event = *jump_backward;
            event->key = compute_final_key(event->key, player);
        }
        else
        {
            ai_enter_walking_backward(ai_context, player, event);
        }
    }
    else if(enemy_status == STATUS_DOWN_HIGH_KICK)
    {
        if(get_rnd_bool(ai_context))
        {
            ai_enter_walking_backward(ai_context, player, event);
        }
        else
        {
            ai_enter_down_defense(ai_context, player, event);
        }
    }
    else if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
        ai_enter_down_defense(ai_context, player, event);
    }
    else
    {
        switch(get_rnd_value(ai_context) % 3)
        {
            case 0:
                ai_context->move = down_low_kick + 1;
                *event = *down_low_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 1:
                ai_context->move = down_high_kick + 1;
                *event = *down_high_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                *event = *sweep;
                event->key = compute_final_key(event->key, player);
                break;
        }
    }
}

static void ai_in_down_low_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    if(is_far_attack(enemy))
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            *event = *jump_backward;
            event->key = compute_final_key(event->key, player);
        }
        else
        {
            ai_enter_walking_backward(ai_context, player, event);
        }
    }
    else if(is_jump_attack(enemy))
    {
        if(get_rnd_bool(ai_context))
        {
            ai_enter_defense(ai_context, player, event);
        }
        else
        {
            ai_enter_walking_forward(ai_context, player, event, IN_RANGE_DOWN_HICH_KICK);
        }
    }
    else if(is_player_in_floor(fight_context, enemy))
    {
        ai_context->move = down_low_kick + 1;
        *event = *down_low_kick;
        event->key = compute_final_key(event->key, player);
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
                    *event = *high_kick;
                    event->key = compute_final_key(event->key, player);
                    break;
                case 1:
                    ai_context->move = high_punch + 1;
                    *event = *high_punch;
                    event->key = compute_final_key(event->key, player);
                    break;
                default:
                    ai_context->move = outward + 1;
                    *event = *outward;
                    event->key = compute_final_key(event->key, player);
            }
        }
    }
}

static void ai_in_punch_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int enemy_status = player_status(player);
    if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
        ai_enter_defense(ai_context, player, event);
    }
    else if(enemy_status == STATUS_SWEEP || enemy_status == STATUS_DOWN_LOW_KICK)
    {
        ai_context->move = jump_backward + 1;
        *event = *jump_backward;
        event->key = compute_final_key(event->key, player);
    }
    else if(enemy_status == STATUS_UPPERCUT || enemy_status == STATUS_LOW_KICK)
    {
        ai_enter_defense(ai_context, player, event);
    }
    else if(enemy_status == STATUS_HIGH_KICK || enemy_status == STATUS_HIGH_PUNCH)
    {
        switch(get_rnd_value(ai_context) % 4)
        {
            case 0:
                ai_enter_defense(ai_context, player, event);
                break;
            case 1:
                ai_context->move = down_low_kick + 1;
                *event = *down_low_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                *event = *sweep;
                event->key = compute_final_key(event->key, player);
                break;
            case 3:
                ai_enter_down_defense(ai_context, player, event);
                break;
        }
    }
    else
    {
        switch(get_rnd_value(ai_context) % 5)
        {
            case 0:
                ai_context->move = low_kick + 1;
                *event = *low_kick;
                break;
            case 1:
                ai_context->move = high_kick + 1;
                *event = *high_kick;
                break;
            case 2:
                ai_context->move = uppercut + 1;
                *event = *uppercut;
                break;
            case 3:
                ai_context->move = high_punch + 1;
                *event = *high_punch;
                break;
            case 4:
                ai_context->move = low_punch + 1;
                *event = *low_punch;
                break;
        }
        event->key = compute_final_key(event->key, player);
    }
}

static void ai_in_kick_range(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
     const int enemy_status = player_status(player);
    if(enemy_status == STATUS_JUMP_KICK || enemy_status == STATUS_JUMP_PUNCH)
    {
        if(get_rnd_bool(ai_context))
        {
            ai_enter_defense(ai_context, player, event);
        }
        else
        {
            ai_context->move = uppercut + 1;
            *event = *uppercut;
            event->key = compute_final_key(event->key, player);
        }
    }
    else if(enemy_status == STATUS_SWEEP || enemy_status == STATUS_DOWN_LOW_KICK)
    {
        if(get_rnd_value(ai_context) % 5 == 0)
        {
            ai_context->move = jump_backward + 1;
            *event = *jump_backward;
            event->key = compute_final_key(event->key, player);
        }
        else
        {
            ai_enter_walking_backward(ai_context, player, event);
        }
    }
    else if(enemy_status == STATUS_UPPERCUT || enemy_status == STATUS_LOW_KICK)
    {
        ai_enter_defense(ai_context, player, event);
    }
    else if(enemy_status == STATUS_HIGH_KICK || enemy_status == STATUS_HIGH_PUNCH)
    {
        switch(get_rnd_value(ai_context) % 4)
        {
            case 0:
                ai_enter_defense(ai_context, player, event);
                break;
            case 1:
                ai_context->move = down_low_kick + 1;
                *event = *down_low_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 2:
                ai_context->move = sweep + 1;
                *event = *sweep;
                event->key = compute_final_key(event->key, player);
                break;
            case 3:
                ai_enter_down_defense(ai_context, player, event);
                break;
        }
    }
    else
    {
        switch(get_rnd_value(ai_context) % 2)
        {
            case 0:
                ai_context->move = low_kick + 1;
                *event = *low_kick;
                event->key = compute_final_key(event->key, player);
                break;
            case 1:
                ai_context->move = high_kick + 1;
                *event = *high_kick;
                event->key = compute_final_key(event->key, player);
                break;
        }
    }
}

static void ai_waiting(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int distance = distance_to_enemy(player, enemy);
    const int current_status = player_status(player);

    if(current_status == STATUS_STANDING)
    {
        if(distance > IN_RANGE_JUMP)
        {
            ai_out_or_range(ai_context, fight_context, player, enemy, event);
        }
        else if(distance < IN_RANGE_JUMP && distance > IN_RANGE_DOWN_LOW_KICK)
        {
            ai_in_jmp_range(ai_context, fight_context, player, enemy, event);
        }
        else if(distance < IN_RANGE_DOWN_LOW_KICK && distance > IN_RANGE_DOWN_HICH_KICK)
        {
            ai_in_down_low_kick_range(ai_context, fight_context, player, enemy, event);
        }
        else if(distance < IN_RANGE_DOWN_HICH_KICK && distance > IN_RANGE_KICK)
        {
            ai_in_down_high_kick_range(ai_context, fight_context, player, enemy, event);
        }
        else if((distance < IN_RANGE_KICK) && (distance > IN_RANGE_PUNCH))
        {
             ai_in_kick_range(ai_context, fight_context, player, enemy, event);
        }
        else if(distance < IN_RANGE_PUNCH)
        {
             ai_in_punch_range(ai_context, fight_context, player, enemy, event);
        }
    }
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

static void ai_walking_forward(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int distance = distance_to_enemy(player, enemy);
    const int enemy_status = player_status(enemy);

    if((distance < ai_context->target)
    || (is_attacking_and_in_range(player, enemy)))
    {
        ai_context->status = AI_STATUS_WAITING;
        event->key = ai_context->key;
        event->status = false;
    }
}

static void ai_defense(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    if(!is_attacking_and_in_range(player, enemy))
    {
        ai_context->status = AI_STATUS_WAITING;
        event->key = PLAYER_INPUT_DEFENSE;
        event->status = false;
    }
}

static void ai_down_defense(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    if(ai_context->counter == 1)
    {
        ai_context->counter = 2;
        event->key = PLAYER_INPUT_DOWN;
        event->status = true;
    }
    else if(ai_context->counter == 2)
    {
        if(!is_attacking_and_in_range(player, enemy))
        {
            ai_context->counter = 3;
            event->key = PLAYER_INPUT_DOWN;
            event->status = false;
        }
    }
    else
    {
        ai_context->status = AI_STATUS_WAITING;
        event->key = PLAYER_INPUT_DEFENSE;
        event->status = false;
    }
}

static void ai_walking_backward(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy, ai_key_event_t* event)
{
    const int distance = distance_to_enemy(player, enemy);
    if(distance < IN_RANGE_PUNCH || distance > IN_RANGE_JUMP)
    {
        ai_context->status = AI_STATUS_WAITING;
        event->key = ai_context->key;
        event->status = false;
    }
}

void ai_think(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy)
{
    if(ai_context->input)
    {
        ai_key_event_t result = {STOP_VALUE, false};
        if(ai_context->move == NULL || ai_context->move->key == STOP_VALUE)
        {
            switch(ai_context->status)
            {
                case AI_STATUS_WAITING:
                    ai_waiting(ai_context, fight_context, player, enemy, &result);
                    break;
                case AI_STATUS_WALKING_FORWARD:
                    ai_walking_forward(ai_context, fight_context, player, enemy, &result);
                    break;
                case AI_STATUS_WALKING_BACKWARD:
                    ai_walking_backward(ai_context, fight_context, player, enemy, &result);
                    break;
                case AI_STATUS_DEFENSE:
                    ai_defense(ai_context, fight_context, player, enemy, &result);
                    break;
                case AI_STATUS_DOWN_DEFENSE:
                    ai_down_defense(ai_context, fight_context, player, enemy, &result);
            }
        }
        else {
            result = *ai_context->move++;
            result.key = compute_final_key(result.key, player);
        }

        if(result.key != STOP_VALUE)
        {
            sw_player_input_set_status(ai_context->input, result.key, result.status);
        }
    }
}

void ai_set_input(ai_context_t* ai_context, player_input_t* input)
{
    if(input != NULL)
    {
        sw_player_input_clear(input);
    }
    ai_context->input = input;
}
