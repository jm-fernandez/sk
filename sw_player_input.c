#include <stdlib.h>
#include <string.h>

#include "sw_player_input.h"
#include "player_input.h"

typedef struct sw_player_input_t_
{
    player_input_t player_input;
    bool input_status[PLAYER_INPUT_COUNT];
}sw_player_input_t;

static bool sw_is_key_pressed(player_input_t* player_input, int key)
{
    sw_player_input_t* sw_player_input = (sw_player_input_t*) player_input;
    return sw_player_input->input_status[key];
}

static void sw_free(player_input_t* player_input)
{
    free(player_input);
}

player_input_t* create_sw_player_input()
{
    sw_player_input_t* result = (sw_player_input_t*) malloc(sizeof(sw_player_input_t));
    if(result)
    {
        memset(result, 0, sizeof(sw_player_input_t));
        result->player_input.is_key_pressed = sw_is_key_pressed;
        result->player_input.free = sw_free;
    }
    return result ? &(result->player_input) : NULL;
}

void sw_player_input_clear(player_input_t* player_input)
{
    int i;
    sw_player_input_t* sw_player_input = (sw_player_input_t*) player_input;

    for(i = 0; i < PLAYER_INPUT_COUNT; ++i)
    {
        sw_player_input->input_status[i] = false;
    }
}

void sw_player_input_set_status(player_input_t* player_input, int key, bool status)
{
    sw_player_input_t* sw_player_input = (sw_player_input_t*) player_input;
    sw_player_input->input_status[key] = status;
}
