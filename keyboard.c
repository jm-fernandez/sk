#include <dos.h>
#include <conio.h>
#include <stddef.h>
#include <stdbool.h>

#include <stdio.h>

#include "keyboard.h"
#include "keyconfig.h"

#define KEYBOARD_INTERRUPT                  9
#define KEY_BUFFER_LEN						32

static void (__interrupt __far * original_keyboard_handler) () = NULL;

static volatile int key_events[KEY_BUFFER_LEN] = {0};
static volatile int last_key = 0;
static volatile int front_pos = 0;
static volatile int back_pos = 0;

static void __interrupt __far keyboard_handler()
{
	const int local_last_key = key_config_map_key(inp(0x60));
	const int new_back = (back_pos + 1) % KEY_BUFFER_LEN;
	if(local_last_key != KEY_CONFIG_UNKNOWN && last_key != local_last_key && new_back != front_pos)
	{
		last_key = local_last_key;
		key_events[back_pos] = local_last_key;
		back_pos = new_back;
	}
	outp(0x20,0x20);
}

bool keyboard_initialize()
{
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

int keyboard_get_key()
{
	int key = KEY_CONFIG_NONE;

	_disable();
	if(front_pos != back_pos)
	{
		key = key_events[front_pos];
		front_pos = (front_pos  + 1) % KEY_BUFFER_LEN;
	}
	_enable();

	return key;
}
