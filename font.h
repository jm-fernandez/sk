#ifndef FONT_H__
#define FONT_H__

#include <stdlib.h>
#include "sprite.h"

typedef struct font_t_ 
{
    const sprite_t* (*get_letter)(const struct font_t_* font, char letter);
    void(*free)(struct font_t_* font);
} font_t;

static const sprite_t* font_get_letter(const font_t* font, char letter)
{
    return font->get_letter(font, letter);
}

static void font_free(font_t* font)
{
    font->free(font);
    font = NULL;
}

font_t* font_load(const char* folder);

#endif // FONT_H__
