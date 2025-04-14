#include <stdlib.h>

#include "keyboard.h"
#include "keyconfig.h"
#include "keyboard_player_input.h"

typedef struct keyboard_player_input_t_
{
    player_input_t player_input;

}keyboard_player_input_t;

static int map_user_input_to_keyboard_key(int key)
{
    int result = KEY_CONFIG_UNKNOWN;
    switch(key)
    {
        case PLAYER_INPUT_UP:
            result = KEY_CONFIG_UP;
            break;
        case PLAYER_INPUT_RIGHT:
            result = KEY_CONFIG_RIGHT;
            break;
        case PLAYER_INPUT_LEFT:
            result = KEY_CONFIG_LEFT;
            break;
        case PLAYER_INPUT_DOWN:
            result = KEY_CONFIG_DOWN;
            break;
        case PLAYER_INPUT_DEFENSE:
            result = KEY_CONFIG_DEFENSE;
            break;
        case PLAYER_INPUT_LOW_PUNCH:
            result = KEY_CONFIG_LOW_PUNCH;
            break;
        case PLAYER_INPUT_HIGH_PUNCH:
            result = KEY_CONFIG_HIGH_PUNCH;
            break;
        case PLAYER_INPUT_LOW_KICK:
            result = KEY_CONFIG_LOW_KICK;
            break;
        case PLAYER_INPUT_HIGH_KICK:
            result = KEY_CONFIG_HIGH_KICK;
            break;
        default:
            /* nothing to do */
            break;
    }
    return result;
} 

#define KEY_CONFIG_HIGH_PUNCH           4
#define KEY_CONFIG_LOW_PUNCH            5
#define KEY_CONFIG_HIGH_KICK            6
#define KEY_CONFIG_LOW_KICK             7
#define KEY_CONFIG_DEFENSE              8

static bool keyboard_is_key_pressed(player_input_t* player_input, int key)
{
    bool result = false;
    int keyboard_key = map_user_input_to_keyboard_key(key);
    if(keyboard_key != KEY_CONFIG_UNKNOWN)
    {
        result = keyboard_get_key_status(keyboard_key, NULL);
    }
    return result;
}

static void keyboard_free(player_input_t* player_input)
{
    free(player_input);
}

player_input_t* create_keyboard_player_input()
{
    keyboard_player_input_t* result = (keyboard_player_input_t*) malloc(sizeof(keyboard_player_input_t));
    if(result)
    {
        result->player_input.is_key_pressed = keyboard_is_key_pressed;
        result->player_input.free = keyboard_free;
    }
    return result ? &(result->player_input) : NULL;
}
