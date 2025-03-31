#ifndef STATE_CTRL_H__
#define STATE_CTRL_H__

#include <stdbool.h>

#include "state.h"

typedef enum game_state_t_ {
    game_state_current = 0,
    game_state_intro = 1,
    game_state_story = 2,
    game_state_pause = 3,
    game_state_fight = 4,
    game_state_count = 5,
} game_state_t;

bool state_ctrl_initialize();
void state_ctrl_deinitialize();

bool state_ctrl_step();

void state_ctrl_set(game_state_t state);

state_t* state_ctrl_get_current();
void state_ctrl_resume_by_pointer(state_t* state);

#endif // STATE_CTRL_H__
