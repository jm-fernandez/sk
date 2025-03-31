#ifndef PLAYER_WALK_H__
#define PLAYER_WALK_H__

#include "player.h"

void player_walk_forward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_walk_backward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);

#endif // PLAYER_WALK_H__
