#ifndef FADE_TO_COLOR_EFFECT_H__
#define FADE_TO_COLOR_EFFECT_H__

#include "effect.h"
#include "palette.h"

effect_t* create_fade_to_color_effect(
    const palette_t* palette,
    const palette_entry_t* target,
    int start_color,
    int end_color,
    unsigned long time);


static inline effect_t* create_fadeout_effect(const palette_t* palette, unsigned long time)
{
    palette_entry_t black = {0, 0, 0, 0};
    return create_fade_to_color_effect(palette, &black, 0, 256, time);
}

#endif // FADE_TO_COLOR_EFFECT_H__

