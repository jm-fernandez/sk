#ifndef SHOW_IMAGE_EFFECT_H__
#define SHOW_IMAGE_EFFECT_H__

#include "util.h"
#include "sprite.h"
#include "render.h"
#include "effect.h"

effect_t* create_show_image_effect(const sprite_t* sprite, const square_t* area, bool clean);

#endif // SHOW_IMAGE_EFFECT_H__
