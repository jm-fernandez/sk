#ifndef KEY_CONFIG__
#define KEY_CONFIG__

#include <stdbool.h>

#define KEY_CONFIG_NONE                 -1

#define KEY_CONFIG_UP                   1
#define KEY_CONFIG_RIGHT                2
#define KEY_CONFIG_LEFT                 3
#define KEY_CONFIG_DOWN                 4
#define KEY_CONFIG_HIGH_PUNCH           5
#define KEY_CONFIG_LOW_PUNCH            6
#define KEY_CONFIG_HIGH_KICK            7
#define KEY_CONFIG_LOW_KICK             8
#define KEY_CONFIG_DEFENSE              9

#define KEY_CONFIG_UP_END               11
#define KEY_CONFIG_RIGHT_END            12
#define KEY_CONFIG_LEFT_END             13
#define KEY_CONFIG_DOWN_END             14
#define KEY_CONFIG_HIGH_PUNCH_END       15
#define KEY_CONFIG_LOW_PUNCH_END        16
#define KEY_CONFIG_HIGH_KICK_END        17
#define KEY_CONFIG_LOW_KICK_END         18
#define KEY_CONFIG_DEFENSE_END          19


#define KEY_CONFIG_PAUSE                64
#define KEY_CONFIG_EXIT                 65
#define KEY_CONFIG_PAUSE_END            66
#define KEY_CONFIG_EXIT_END             67


#define KEY_CONFIG_UNKNOWN              128


void key_config_init();
int key_config_map_key(int scancode);

#endif // KEY_CONFIG__
