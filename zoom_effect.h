#ifndef ZOOM_EFFECT_H__
#define ZOOM_EFFECT_H__

#include "util.h"
#include "sprite.h"
#include "effect.h"

effect_t* create_zoom_effect(const sprite_t* sprite, const square_t* start, const square_t* stop, int time, bool clean);

#endif // ZOOM_EFFECT_H__
