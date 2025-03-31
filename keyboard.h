#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdbool.h>

#define SCANCODE_NONE            0

#define SCANCODE_ESC             1
#define SCANCODE_P               25
#define SCANCODE_A               30
#define SCANCODE_S               31
#define SCANCODE_D               32
#define SCANCODE_X               45
#define SCANCODE_C               46

#define SCANCODE_ARROW_UP        72
#define SCANCODE_ARROW_LEFT      75
#define SCANCODE_ARROW_RIGHT     77
#define SCANCODE_ARROW_DOWN      80

#define SCANCODE_ESC_R           129
#define SCANCODE_P_R             153
#define SCANCODE_A_R             158
#define SCANCODE_S_R             159
#define SCANCODE_D_R             160
#define SCANCODE_X_R             173
#define SCANCODE_C_R             174

#define SCANCODE_ARROW_UP_R      200
#define SCANCODE_ARROW_LEFT_R    203
#define SCANCODE_ARROW_RIGHT_R   205
#define SCANCODE_ARROW_DOWN_R    208

bool keyboard_initialize();
void keyboard_deinitialize();

int keyboard_get_key();

#endif // KEYBOARD_H_
