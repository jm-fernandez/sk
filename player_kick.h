#ifndef PLAYER_KICK_H__
#define PLAYER_KICK_H__

#include "player.h"

void player_low_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_high_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_outward_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_down_low_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_down_high_kick_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);
void player_sweep_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy);

#endif // PLAYER_KICK_H__
