#ifndef PALETTE_TRANSITION_EFFECT_H__
#define PALETTE_TRANSITION_EFFECT_H__

#include "effect.h"
#include "palette.h"
#include "render.h"

effect_t* create_palette_transition_effect(
    const palette_t* src,
    const palette_t* dst,
    int start_color,
    int end_color,
    int time);

#endif // PALETTE_TRANSITION_EFFECT_H__

