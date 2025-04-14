#include <dos.h>
#include <conio.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

#include "keyboard.h"
#include "keyconfig.h"

#define KEYBOARD_INTERRUPT                  9

static void (__interrupt __far * original_keyboard_handler) () = NULL;

static bool static_key_status[KEY_CONFIG_COUNT] = {false};
static int static_key_count[KEY_CONFIG_COUNT] = {0};

static void __interrupt __far keyboard_handler()
{
	const int key = key_config_map_key(inp(0x60));
    if(key != KEY_CONFIG_UNKNOWN)
    {
        const bool new_value = key >= KEY_CONFIG_COUNT ? false : true;
        const int key_index = key >= KEY_CONFIG_COUNT ? key - KEY_CONFIG_COUNT : key;

        if(static_key_status[key_index] != new_value)
        {
            static_key_status[key_index] = new_value;
			if(new_value)
			{
				static_key_count[key_index] += 1;
			}
        }
    }
	outp(0x20,0x20);
}

bool keyboard_initialize()
{
	int i;
	for(i = 0; i < KEY_CONFIG_COUNT; ++i)
	{
		static_key_status[i] = false;
		static_key_count[i] = 0;
	}

	_disable();
	original_keyboard_handler = _dos_getvect(KEYBOARD_INTERRUPT);
	_dos_setvect(KEYBOARD_INTERRUPT, keyboard_handler);
	_enable();
  	return true;
}

void keyboard_deinitialize()
{
	_disable();
	_dos_setvect(KEYBOARD_INTERRUPT, original_keyboard_handler);
	_enable();
}

bool keyboard_get_key_status(int key, int* key_count)
{
	bool result = false;

	_disable();
	result = static_key_status[key];
	if(key_count)
	{
		*key_count = static_key_count[key];
	}
	_enable();
	return result;
}
