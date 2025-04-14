#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdbool.h>

#define SCANCODE_RELEASE_FLAG     128

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

#define SCANCODE_ESC_R           (SCANCODE_ESC + SCANCODE_RELEASE_FLAG)
#define SCANCODE_P_R             (SCANCODE_P + SCANCODE_RELEASE_FLAG)
#define SCANCODE_A_R             (SCANCODE_A + SCANCODE_RELEASE_FLAG)
#define SCANCODE_S_R             (SCANCODE_S + SCANCODE_RELEASE_FLAG)
#define SCANCODE_D_R             (SCANCODE_D + SCANCODE_RELEASE_FLAG)
#define SCANCODE_X_R             (SCANCODE_X + SCANCODE_RELEASE_FLAG)
#define SCANCODE_C_R             (SCANCODE_C + SCANCODE_RELEASE_FLAG)

#define SCANCODE_ARROW_UP_R      (SCANCODE_ARROW_UP + SCANCODE_RELEASE_FLAG)
#define SCANCODE_ARROW_LEFT_R    (SCANCODE_ARROW_LEFT + SCANCODE_RELEASE_FLAG)
#define SCANCODE_ARROW_RIGHT_R   (SCANCODE_ARROW_RIGHT + SCANCODE_RELEASE_FLAG)
#define SCANCODE_ARROW_DOWN_R    (SCANCODE_ARROW_DOWN + SCANCODE_RELEASE_FLAG)

bool keyboard_initialize();
void keyboard_deinitialize();

bool keyboard_get_key_status(int key, int* key_count);

#endif // KEYBOARD_H_
