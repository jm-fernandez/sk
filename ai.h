#ifndef AI_H__
#define AI_H__

#include "player.h"

#define RND_COUNT 32

typedef struct ai_context_t_
{
    int rnd[RND_COUNT];
    int rnd_index;
    int status;
    int key;
    int target;
    int counter;
    const int* move;
} ai_context_t;

void ai_init(ai_context_t* ai_context);

int ai_last_key(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy);

#endif // AI_H__
