#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "render.h"
#include "sprite.h"
#include "util.h"
#include "svga.h"

#define RENDER_MAX_DIRTY_AREAS 50

typedef struct video_mode_info_t_ {
    unsigned int x_resolution;
    unsigned int y_resolution;
    unsigned int bpp;
} video_mode_info_t;

typedef enum render_active_view_t_ {
    render_active_view_1 = 1,
    render_active_view_2 = 2,
}render_active_view_t;

typedef struct render_element_t_
{
    square_t square;
    bool flipped;
    unsigned long sprite_id;
}rendered_element_t;

typedef struct render_view_t
{
    square_t dirty_areas[RENDER_MAX_DIRTY_AREAS];
    int dirty_area_count;
    unsigned char* ptr;
    unsigned int scanline_start;
} render_view_t;

typedef struct render_t_
{
    SVGAMode svga_context;
    palette_t palette;
    palette_info_t palette_info;
    bool palette_modified;
    bool view_modified;
    unsigned int scanline_offset;
    unsigned int scanline_length;
    const sprite_t* background;
    render_view_t view_1;
    render_view_t view_2;
    render_view_t* active_view;
    render_view_t* background_view;
} render_t;

static bool check_video_mode(SVGAMode* info, void* user_data) {
    const video_mode_info_t* video_mode_info = (const video_mode_info_t*) user_data;
    return (info->XResolution == video_mode_info->x_resolution) &&
                (info->YResolution == video_mode_info->y_resolution) &&
                (info->BitsPerPixel == video_mode_info->bpp);
}


static render_t render;

bool render_initialize(unsigned int x_resolution, unsigned int y_resolution, unsigned int bpp)
{
    bool result = false;
    bool video_mode_found = false;
    video_mode_info_t video_mode_info = {0};
    SVGAMode video_info = {0};

    video_mode_info.x_resolution = x_resolution;
    video_mode_info.y_resolution = y_resolution;
    video_mode_info.bpp = bpp;

    video_mode_found = find_video_mode(&video_info, check_video_mode, &video_mode_info);
    if(video_mode_found && set_super_vga_video_mode(video_info.mode))
    {
        render.svga_context = video_info;
        render.background = NULL;
        render.scanline_offset = 0;
        render.scanline_length = x_resolution;
        memset(&(render.palette), 0, sizeof(render.palette));
        render.palette_modified = false;
        render.view_1.dirty_area_count = 0;
        render.view_1.ptr = video_info.BasePtr;
        render.view_1.scanline_start = 0;
        render.view_2.dirty_area_count = 0;
        render.view_2.ptr = video_info.BasePtr + x_resolution * y_resolution;
        render.view_2.scanline_start = y_resolution;
        render.active_view = &(render.view_1);
        render.background_view = &(render.view_2);

        set_palette_format_8_bits();
        result = true;
    }
    return result;
}

void render_deinitialize()
{
    set_super_vga_video_mode(0x03);
}

static void render_from_last_frame(square_t* square)
{
  const unsigned char* src = render.active_view->ptr + square->y * render.scanline_length + square->x;
  unsigned char* dst = render.background_view->ptr + square->y * render.scanline_length + square->x;
  int i = 0;
  for(;i< square->sy; ++i)
  {
    memccpy(dst, src, 1, square->sx);
    src += render.scanline_length;
    dst += render.scanline_length;
  }
  render.background_view->dirty_areas[render.background_view->dirty_area_count++] = *square;
}

void render_last_frame()
{
    render_view_t* view = render.active_view;
    int i = 0;
    for(; i < view->dirty_area_count; ++i)
    {
      render_from_last_frame(&(view->dirty_areas[i]));
    }
    render.view_modified = true;  
}

static void set_background(const sprite_t* sprite)
{
  int lines = 0;
  const unsigned int height = render.svga_context.YResolution;
  unsigned char* screen_ptr_1 = render.view_1.ptr;
  unsigned char* screen_ptr_2 = render.view_2.ptr;
  const unsigned char* sprite_ptr = sprite_first_stride(sprite);
  const unsigned int line_len = sprite_width(sprite);

  for(; lines < height; ++lines)
  {
    memcpy(screen_ptr_1, sprite_ptr, line_len);
    memcpy(screen_ptr_2, sprite_ptr, line_len);
    screen_ptr_1 += render.scanline_length;
    screen_ptr_2 += render.scanline_length;
    sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
  }
}

static void clean_background(unsigned char* ptr, unsigned int scanline, const sprite_t* sprite, square_t region)
{
    int y0 = 0;
    const unsigned char* sprite_stride = sprite_first_stride(sprite);
    while(y0++ < region.y)
    {
        ptr += scanline;
        sprite_stride = sprite_next_stride(sprite, sprite_stride);
    }

    y0 = 0;
    while(y0++ < region.sy)
    {
        const unsigned char* sprite_line_start = sprite_stride + region.x;
        unsigned char* screen_line_start = ptr + region.x;

        memcpy(screen_line_start, sprite_line_start, region.sx);
        sprite_stride = sprite_next_stride(sprite, sprite_stride);
        ptr += scanline;
    }
}

bool render_set_background(const sprite_t* background)
{
    bool result = false;
    const unsigned int height = sprite_height(background);
    if(height == render.svga_context.YResolution)
    {
        const unsigned int width = sprite_width(background);
        if(width >= render.svga_context.XResolution)
        {
            if(width != render.scanline_length)
            {
                render.scanline_length = set_logical_scanline_length(width);
                render.view_2.ptr = render.svga_context.BasePtr +
                    render.scanline_length * render.svga_context.YResolution;
            }
            
            set_background(background);
            render.background = background;
            render.view_1.dirty_area_count = 0;
            render.view_2.dirty_area_count = 0;
            result = true;
        }
    }
    return result;
}

const sprite_t* render_get_background()
{
  return render.background;
}

void render_set_palette(const palette_t* palette)
{
  render.palette = *palette;
  render.palette_modified = true;
}

const palette_t* render_get_palette()
{
  return &(render.palette);
}


void render_clean()
{
    render_view_t* view = render.background_view;
    int i = 0;
    for(; i < view->dirty_area_count; ++i)
    {
      clean_background(view->ptr, render.scanline_length, render.background, view->dirty_areas[i]);
    }
    view->dirty_area_count = 0;
    render.view_modified = true;
}

static void render_sprite_shadow_flipped(const sprite_t* sprite, const square_t* square)
{
  int lines = 0;
  unsigned char* screen_ptr = render.background_view->ptr + square->x + square->y * render.scanline_length;

  const unsigned int spr_height = sprite_height(sprite);
  const unsigned int spr_width = sprite_width(sprite);

  const int y_step = spr_height / square->sy;
  int y_pos = 0;

  const unsigned char* sprite_ptr = sprite_first_stride(sprite);
  const unsigned char transparent_color = sprite_transparent_color(sprite);
  unsigned int cur_pos = 0;

  for(; lines < square->sy; ++lines)
  {
    unsigned int x_pos_dest = 0;
    

    y_pos += y_step;
    while(y_pos > cur_pos)
    {
      sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
      ++cur_pos;
    }

    for(; x_pos_dest < square->sx; ++x_pos_dest)
    {
      const unsigned char pixel = sprite_ptr[spr_width - x_pos_dest - 1];
      if(pixel != transparent_color)
      {
          const int background_index = render.palette_info.background_index;
          const int shadow_index = render.palette_info.background_shadow_index;
          screen_ptr[x_pos_dest] += (shadow_index - background_index);
      }
    }
    screen_ptr += render.scanline_length;
  }

}

static void render_sprite_shadow(const sprite_t* sprite, const square_t* square)
{
  int lines = 0;
  unsigned char* screen_ptr = render.background_view->ptr + square->x + square->y * render.scanline_length;

  const unsigned int spr_height = sprite_height(sprite);
  const unsigned int spr_width = sprite_width(sprite);

  int y_step = spr_height / square->sy;
  int y_pos = 0;

  const unsigned char* sprite_ptr = sprite_first_stride(sprite);
  const unsigned char transparent_color = sprite_transparent_color(sprite);
  unsigned int cur_pos = 0;

  for(; lines < square->sy; ++lines)
  {
    unsigned int x_pos_dest = 0;
        
    y_pos += y_step;

    while(y_pos > cur_pos)
    {
      sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
      ++cur_pos;
    }

    for(; x_pos_dest < square->sx; ++x_pos_dest)
    {
      const unsigned char pixel = sprite_ptr[x_pos_dest];
      if(pixel != transparent_color)
      {
          const int background_index = render.palette_info.background_index;
          const int shadow_index = render.palette_info.background_shadow_index;
          screen_ptr[x_pos_dest] += (shadow_index - background_index);
      }
    }
    screen_ptr += render.scanline_length;
  }
}

static void render_sprite_scaled(const sprite_t* sprite, const square_t* square, bool flipped)
{
  int lines = 0;
  unsigned char* screen_ptr = render.background_view->ptr + square->x + square->y * render.scanline_length;

  const unsigned int spr_height = sprite_height(sprite);
  const unsigned int spr_width = sprite_width(sprite);

  double y_step = (double)spr_height / square->sy;
  double x_step = (double)spr_width / square->sx;
  double y_pos_dbl = 0.0;

  const unsigned char* sprite_ptr = sprite_first_stride(sprite);
  const unsigned char transparent_color = sprite_transparent_color(sprite);
  unsigned int cur_pos = 0;

  for(; lines < square->sy; ++lines)
  {
    double x_pos_dbl = 0.0;
    unsigned int y_pos = 0;
    unsigned int x_pos_dest = 0;
    

    y_pos_dbl += y_step;
    y_pos = (unsigned int) y_pos_dbl;
    if(y_pos >= spr_height)
    {
      y_pos = spr_height - 1;
    }

    while(y_pos > cur_pos)
    {
      sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
      ++cur_pos;
    }

    for(; x_pos_dest < square->sx; ++x_pos_dest)
    {
      unsigned char pixel = 0;
      unsigned int x_pos = 0;
      x_pos_dbl += x_step;
      x_pos = (unsigned int) x_pos_dbl;
      if(x_pos >= spr_width)
      {
        x_pos = spr_width - 1;
      }

      pixel = sprite_ptr[flipped ? spr_width - x_pos - 1 : x_pos];
      if(pixel != transparent_color)
      {
          screen_ptr[x_pos_dest] = pixel;
      }
    }
    screen_ptr += render.scanline_length;
  }
}


static void render_sprite_at_flipped(const sprite_t* sprite, int x_pos, int y_pos)
{
  int lines = 0;
  unsigned char* screen_ptr = render.background_view->ptr + y_pos * render.scanline_length + x_pos;
  const unsigned int spr_width = sprite_width(sprite);
  const unsigned int spr_height = sprite_height(sprite);
  const unsigned char* sprite_ptr = sprite_first_stride(sprite);

  const unsigned char transparent_color = sprite_transparent_color(sprite);

  for(; lines < spr_height; ++lines)
  {
    int xpos = 0;
    unsigned char* screen_stride = screen_ptr;
    const unsigned char* sprite_stride = sprite_ptr;
    for(; xpos < spr_width; ++xpos)
    {
        const unsigned char pixel = sprite_stride[spr_width - xpos - 1];
        if(pixel != transparent_color)
        {
            screen_stride[xpos] = pixel;
        }
    }
    screen_ptr += render.scanline_length;
    sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
  }
}

static void render_sprite_at(const sprite_t* sprite, int x_pos, int y_pos)
{
  int lines = 0;
  unsigned char* screen_ptr = render.background_view->ptr + y_pos * render.scanline_length + x_pos;
  const unsigned int spr_width = sprite_width(sprite);
  const unsigned int spr_height = sprite_height(sprite);
  const unsigned char* sprite_ptr = sprite_first_stride(sprite);

  const unsigned char transparent_color = sprite_transparent_color(sprite);

  for(; lines < spr_height; ++lines)
  {
    int xpos = 0;
    unsigned char* screen_stride = screen_ptr;
    const unsigned char* sprite_stride = sprite_ptr;
    for(; xpos < spr_width; ++xpos)
    {
        const unsigned char pixel = sprite_stride[xpos];
        if(pixel != transparent_color)
        {
            screen_stride[xpos] = pixel;
        }
    }
    screen_ptr += render.scanline_length;
    sprite_ptr = sprite_next_stride(sprite, sprite_ptr);
  }
}

static void update_dirty_area_list(const square_t* square)
{
  if(square->sx > 0 && square->sy > 0)
  {
    int i = 0;

    for(; i < render.background_view->dirty_area_count; ++i)
    {
      if(square_contained(&(render.background_view->dirty_areas[i]), square))
      {
        break;
      }
      else if(square_contained(square, &(render.background_view->dirty_areas[i])))
      {
        render.background_view->dirty_areas[i] = *square;
        break;
      }
    }

    if(i == render.background_view->dirty_area_count)
    {
      ++render.background_view->dirty_area_count;
      render.background_view->dirty_areas[i] = *square;
    }

  }
}

static void render_sprite_internal(const sprite_t* sprite, const square_t* square, bool flipped, bool shadow)
{
  if(square->sx > 0 && square->sy > 0)
  {
    const unsigned int width = sprite_width(sprite);
    const unsigned int height = sprite_height(sprite);

    update_dirty_area_list(square);

    if(width == square->sx && height == square->sy)
    {
      if(flipped)
      {
        render_sprite_at_flipped(sprite, square->x, square->y);
      }
      else
      {
        render_sprite_at(sprite, square->x, square->y);
      }
    }
    else
    {
      if(shadow)
      {
        if(flipped)
        {
          render_sprite_shadow_flipped(sprite, square);
        }
        else
        {
          render_sprite_shadow(sprite, square);
        }
      }
      else
      {
        render_sprite_scaled(sprite, square, flipped);
      }
    }
    render.view_modified = true;
  }
}

void render_square(const square_t* square, int color, int fill_color)
{
  if(square->sx > 0 && square->sy > 0)
  {
    unsigned int y = square->y;
    const unsigned int x = square->x;
    const unsigned int x_end = x + square->sx;
    const unsigned int y_end = y + square->sy;
    unsigned char* screen_ptr = render.background_view->ptr + y * render.scanline_length + x;
    unsigned char* screen_line = screen_ptr;
    unsigned char* last_screen_line = screen_ptr + (square->sy - 1) * render.scanline_length;
    unsigned int x_pos = x;

    update_dirty_area_list(square);

    while(x_pos < x_end)
    {
      *screen_line++= color;
      *last_screen_line++ = color;
      ++x_pos;
    }

    ++y;
    screen_ptr += render.scanline_length;

    while(y < y_end - 1)
    {
      unsigned char* screen_line = screen_ptr;
      unsigned int x_pos = x;
      
      *screen_line= color;
      screen_line[square->sx - 1] = color;
      ++x_pos;

      if(fill_color != SQUARE_NO_FILL)
      {
        ++screen_line;
        while(x_pos < x_end - 1)
        {
          *screen_line++= fill_color;
          ++x_pos;
        }
      }

      screen_ptr += render.scanline_length;
      ++y;      
    }
  }
}

void render_sprite(const sprite_t* sprite, const square_t* square, bool flipped)
{
  render_sprite_internal(sprite, square, flipped, false);
}

void render_shadow(const sprite_t* sprite, const square_t* square, bool flipped)
{
  render_sprite_internal(sprite, square, flipped, true);
}

void render_show(bool wait_for_retrace)
{
  bool retrace_sync = render.view_modified;
  if(render.view_modified)
  {
    render_view_t* render_view = render.background_view;
    set_display_start(&render.svga_context, render.scanline_offset, render_view->scanline_start, true);
    render.background_view = render.active_view;
    render.active_view = render_view;
    render.view_modified = false;
  }

  if(render.palette_modified)
  {
    set_palette_data(&(render.palette));
    render.palette_modified = false;
  }
}

unsigned int render_get_view_offset()
{
  return render.scanline_offset;
}


void render_set_view_offset(unsigned int offset)
{
  if(render.scanline_offset != offset)
  {
    render.scanline_offset = offset;
    render.view_modified = true;
  }
}

unsigned int render_get_scanline_length()
{
  return render.scanline_length;
}

void render_set_palette_info(palette_info_t* palette_info)
{
  render.palette_info = *palette_info;
}

const palette_info_t* render_get_palette_info()
{
  return &(render.palette_info);
}
