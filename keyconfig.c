#include "keyconfig.h"
#include "keyboard.h"

static int configured[256];

void key_config_init()
{
    int i = 0;
    for(; i < 256; ++i)
    {
        configured[i] = KEY_CONFIG_UNKNOWN;
    }

    configured[SCANCODE_NONE] = KEY_CONFIG_NONE;
    configured[SCANCODE_ESC] = KEY_CONFIG_EXIT;
    configured[SCANCODE_ESC_R] = KEY_CONFIG_EXIT_END;
    configured[SCANCODE_P] = KEY_CONFIG_PAUSE;
    configured[SCANCODE_P_R] = KEY_CONFIG_PAUSE_END;

    configured[SCANCODE_A] = KEY_CONFIG_DEFENSE;
    configured[SCANCODE_S] = KEY_CONFIG_HIGH_KICK;
    configured[SCANCODE_D] = KEY_CONFIG_HIGH_PUNCH;
    configured[SCANCODE_X] = KEY_CONFIG_LOW_KICK;
    configured[SCANCODE_C] = KEY_CONFIG_LOW_PUNCH;

    configured[SCANCODE_ARROW_UP] = KEY_CONFIG_UP;
    configured[SCANCODE_ARROW_LEFT] = KEY_CONFIG_LEFT;
    configured[SCANCODE_ARROW_RIGHT] = KEY_CONFIG_RIGHT;
    configured[SCANCODE_ARROW_DOWN] = KEY_CONFIG_DOWN;

    configured[SCANCODE_A_R] = KEY_CONFIG_DEFENSE_END;
    configured[SCANCODE_S_R] = KEY_CONFIG_HIGH_KICK_END;
    configured[SCANCODE_D_R] = KEY_CONFIG_HIGH_PUNCH_END;
    configured[SCANCODE_X_R] = KEY_CONFIG_LOW_KICK_END;
    configured[SCANCODE_C_R] = KEY_CONFIG_LOW_PUNCH_END;

    configured[SCANCODE_ARROW_UP_R] = KEY_CONFIG_UP_END;
    configured[SCANCODE_ARROW_LEFT_R] = KEY_CONFIG_LEFT_END;
    configured[SCANCODE_ARROW_RIGHT_R] = KEY_CONFIG_RIGHT_END;
    configured[SCANCODE_ARROW_DOWN_R] = KEY_CONFIG_DOWN_END;
}

void key_config_deinit()
{
}

int key_config_map_key(int scancode)
{
    return configured[scancode];
}
