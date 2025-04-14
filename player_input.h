#ifndef PLAYER_INPUT_H__
#define PLAYER_INPUT_H__

#include <stdbool.h>

#define PLAYER_INPUT_UP                 0
#define PLAYER_INPUT_RIGHT              1
#define PLAYER_INPUT_LEFT               2
#define PLAYER_INPUT_DOWN               3
#define PLAYER_INPUT_DEFENSE            4
#define PLAYER_INPUT_LOW_PUNCH          5
#define PLAYER_INPUT_HIGH_PUNCH         6
#define PLAYER_INPUT_LOW_KICK           7
#define PLAYER_INPUT_HIGH_KICK          8
#define PLAYER_INPUT_COUNT              9

typedef struct player_input_t_
{
    bool (*is_key_pressed)(struct player_input_t_* player_input, int key);
    void (*free)(struct player_input_t_* player_input);
}player_input_t;

static bool player_input_is_key_pressed(player_input_t* player_input, int key)
{
    return player_input->is_key_pressed(player_input, key);
}

static void player_input_free(player_input_t* player_input)
{
    player_input->free(player_input);
}
#endif // PLAYER_INPUT_H__
