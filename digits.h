#ifndef DIGITS_H__
#define DIGITS_H__

#include "sprite.h"

#define DIGITS_COUNT           10

typedef struct digits_t_
{
    sprite_t* sprites[DIGITS_COUNT];
} digits_t;

digits_t* digits_create(const char* folder_path);
void digits_free(digits_t* digits);

static const sprite_t* digits_get(const digits_t* digits, int digit)
{
    return digits->sprites[digit];
}

#endif // DIGITS_H__
