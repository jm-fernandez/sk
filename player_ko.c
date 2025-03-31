#include "player_ko.h"
#include "player.h"

static void player_ko_advance(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
}

void player_ko_enter(const fight_context_t* fight_context, player_t* player, player_t* enemy)
{
  player->status = STATUS_KO;
  player->advance = player_ko_advance;
}

