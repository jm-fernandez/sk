#ifndef SW_PLAYER_INPUT_H__
#define SW_PLAYER_INPUT_H__

#include "player_input.h"

player_input_t* create_sw_player_input();

void sw_player_input_clear(player_input_t* player_input);
void sw_player_input_set_status(player_input_t* player_input, int key, bool status);

#endif // SW_PLAYER_INPUT_H__
