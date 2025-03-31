#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sprite.h"

typedef struct solid_sprite_t_
{
    sprite_t sprite;
    unsigned int width;
    unsigned int height;
    palette_t palette;
    unsigned char* data;
} solid_sprite_t;

static unsigned int solid_width(const sprite_t* sprite)
{
    const solid_sprite_t* solid_sprite = (const solid_sprite_t*) sprite;
    return solid_sprite->width;
}

static unsigned int solid_height(const sprite_t* sprite)
{
    const solid_sprite_t* solid_sprite = (const solid_sprite_t*) sprite;
    return solid_sprite->height;
}

static unsigned int solid_color_count(const sprite_t* sprite)
{
    return 1U;
}

static const palette_t* solid_palette(const sprite_t* sprite)
{
    const solid_sprite_t* solid_sprite = (const solid_sprite_t*) sprite;
    return &(solid_sprite->palette);
}

static const unsigned char* solid_first_stride(const sprite_t* sprite)
{
    const solid_sprite_t* solid_sprite = (const solid_sprite_t*) sprite;
    return solid_sprite->data;
}

static const unsigned char* solid_next_stride(const sprite_t* sprite, const unsigned char* stride)
{
    return stride;
}

static const unsigned char* solid_get_stride(const sprite_t* sprite, unsigned int y)
{
    return solid_first_stride(sprite);
}

static unsigned char solid_transparent_color(const sprite_t* sprite)
{
    return 1;
}

static void solid_free(sprite_t* sprite)
{
    solid_sprite_t* solid_sprite = (solid_sprite_t*) sprite;
    if(solid_sprite)
    {
        if(solid_sprite->data)
        {
            free(solid_sprite->data);
        }
        free(solid_sprite);
        solid_sprite = NULL;
    }
}

sprite_t* create_solid_sprite(unsigned int width, unsigned int height, palette_entry_t color)
{
    solid_sprite_t* result = (solid_sprite_t*) malloc(sizeof(solid_sprite_t));
    if(result)
    {
      int i = 0;

      result->width = width;
      result->height = height;

      for(; i < 256; ++i)
      {
        result->palette.colors[i] = color;
      }
      
      result->data = (unsigned char*) malloc(width);
      memset(result->data, 0, width);

      result->sprite.width = solid_width;
      result->sprite.height = solid_height;
      result->sprite.color_count = solid_color_count;
      result->sprite.palette = solid_palette;
      result->sprite.first_stride = solid_first_stride;
      result->sprite.next_stride = solid_next_stride;
      result->sprite.free = solid_free;
      result->sprite.get_stride = solid_get_stride;
      result->sprite.transparent_color = solid_transparent_color;

    }
    return result == NULL ? NULL : &(result->sprite);
}
