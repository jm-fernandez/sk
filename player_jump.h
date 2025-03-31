#ifndef PLAYER_JUMP_H__
#define PLAYER_JUMP_H__

#include "player.h"

void player_jump_forward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_jump_backward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_jump_vertical_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);

#endif // PLAYER_JUMP_H__
