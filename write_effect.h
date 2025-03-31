#ifndef WRITE_EFFECT_H__
#define WRITE_EFFECT_H__

#include "effect.h"
#include "font.h"

effect_t* create_write_effect(const font_t* font, const char* text, int x, int y, int size, unsigned long time, bool clean);

#endif // WRITE_EFFECT_H__
