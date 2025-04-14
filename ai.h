#ifndef AI_H__
#define AI_H__

#include <stdbool.h>

#include "player.h"
#include "player_input.h"

#define RND_COUNT 64

typedef struct ai_key_event_t_
{
    int key;
    bool status;
} ai_key_event_t;

typedef struct ai_context_t_
{
    player_input_t* input;
    int rnd[RND_COUNT];
    int rnd_index;
    int status;
    int key;
    int target;
    int counter;
    const ai_key_event_t* move;
} ai_context_t;

void ai_init(ai_context_t* ai_context);

// it has to be a sw_player_input instance. It can be NULL
void ai_set_input(ai_context_t* ai_context, player_input_t* input);

void ai_think(ai_context_t* ai_context, const fight_context_t* fight_context, const player_t* player, const player_t* enemy);

#endif // AI_H__
