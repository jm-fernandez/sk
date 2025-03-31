#ifndef SPRITE_H__
#define SPRITE_H__

#include "palette.h"

typedef struct sprite_t_
{
  unsigned int (*width)(const struct sprite_t_* sprite);
  unsigned int (*height)(const struct sprite_t_* sprite);
  unsigned int (*color_count)(const struct sprite_t_* sprite);
  const palette_t* (*palette)(const struct sprite_t_* sprite);
  unsigned char (*transparent_color)(const struct sprite_t_* sprite);
  const unsigned char* (*first_stride)(const struct sprite_t_* sprite);
  const unsigned char* (*next_stride)(const struct sprite_t_* sprite, const unsigned char* stride);
  const unsigned char* (*get_stride)(const struct sprite_t_* sprite, unsigned int y);
  void (*free)(struct sprite_t_* sprite);
} sprite_t;

static inline int sprite_width(const sprite_t* sprite)
{
  return sprite->width(sprite);
}

static inline int sprite_height(const sprite_t* sprite)
{
  return sprite->height(sprite);
}

static inline unsigned int sprite_color_count(const sprite_t* sprite)
{
  return sprite->color_count(sprite);
}

static inline const palette_t* sprite_palette(const sprite_t* sprite)
{
  return sprite->palette(sprite);
}

static inline const unsigned char* sprite_first_stride(const sprite_t* sprite)
{
  return sprite->first_stride(sprite);
}

static inline const unsigned char* sprite_next_stride(const sprite_t* sprite, const unsigned char* stride)
{
  return sprite->next_stride(sprite, stride);
}

static inline const unsigned char* sprite_get_stride(const sprite_t* sprite, unsigned int y)
{
  return sprite->get_stride(sprite, y);
}

static inline unsigned char sprite_transparent_color(const struct sprite_t_* sprite)
{
  return sprite->transparent_color(sprite);
}

static inline void sprite_free(struct sprite_t_* sprite)
{
  sprite->free(sprite);
}

sprite_t* create_bmp_sprite(const char *filename);
sprite_t* create_solid_sprite(unsigned int width, unsigned int height, palette_entry_t color);


#endif // SPRITE_H__
