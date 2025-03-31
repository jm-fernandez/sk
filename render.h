#ifndef RENDER_H__
#define RENDER_H__

#include <stdbool.h>

#include "sprite.h"
#include "palette.h"
#include "util.h"

// to render squeres without filling 
#define SQUARE_NO_FILL 256

typedef struct palette_info_t_  {
    int foreground_index;
    int foreground_count;
    int background_index;
    int background_count;
    int background_shadow_index;
    int background_shadow_count;
    int extra_index;
    int extra_count;
}palette_info_t;

bool render_initialize(unsigned int x_resolution, unsigned int y_resolution, unsigned int bpp);

void render_deinitialize();

bool render_set_background(const sprite_t* background);
const sprite_t* render_get_background();

void render_clean();

void render_set_palette(const palette_t* palette);
const palette_t* render_get_palette();

void render_set_palette_info(palette_info_t* palette_info);
const palette_info_t* render_get_palette_info();

void render_sprite(const sprite_t* sprite, const square_t* square, bool flipped);
void render_shadow(const sprite_t* sprite, const square_t* square, bool flipped);

void render_square(const square_t* square, int color, int fill_collor);

unsigned int render_get_scanline_length();

unsigned int render_get_view_offset();
void render_set_view_offset(unsigned int offset);

void render_last_frame();
void render_show(bool wait_for_retrace);

#endif // RENDER_H__
