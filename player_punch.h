#ifndef PLAYER_PUNCH_H__
#define PLAYER_PUNCH_H__

#include "player.h"

void player_high_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_low_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_down_punch_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);

#endif // PLAYER_PUNCH_H__
