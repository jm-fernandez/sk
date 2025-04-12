#include <stdlib.h>
#include <stdio.h>

#include "sprite.h"

typedef struct bmp_sprite_t_
{
    sprite_t sprite;
    unsigned int width;
    unsigned int height;
    unsigned int colorCount;
    palette_t palette;
    unsigned char* data;
} bmp_sprite_t;

#pragma pack(push, 1)
typedef struct BmpFileHeader_ {
   char bfType[2];
   unsigned int bfSize;
   unsigned short int bfReserved1;
   unsigned short int bfReserved2;
   unsigned long int bfOffBits;
} BmpFileHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BmpImageHeader_ {
   unsigned int biSize;
   int biWidth;
   int biHeight;
   unsigned short int biPlanes;
   unsigned short int biBitCount;
   unsigned int biCompression;
   unsigned int biSizeImage;
   int biXPelsPerMeter;
   int biYPelPerMeter;
   unsigned int biClrUsed;
   unsigned int biClrImportant;  
} BmpImageHeader;
#pragma pack(pop)

static unsigned int bmp_width(const sprite_t* sprite)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return bmp_sprite->width;
}

static unsigned int bmp_height(const sprite_t* sprite)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return bmp_sprite->height;
}

static unsigned int bmp_color_count(const sprite_t* sprite)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return bmp_sprite->colorCount;
}

static const palette_t* bmp_palette(const sprite_t* sprite)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return &(bmp_sprite->palette);
}

static const unsigned char* bmp_first_stride(const sprite_t* sprite)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return bmp_sprite->data + (bmp_sprite->height - 1) * ((bmp_sprite->width + 3) & ~3);
}

static const unsigned char* bmp_next_stride(const sprite_t* sprite, const unsigned char* stride)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return stride  - ((bmp_sprite->width + 3) & ~3);
}

static const unsigned char* bmp_get_stride(const sprite_t* sprite, unsigned int y)
{
    const bmp_sprite_t* bmp_sprite = (const bmp_sprite_t*) sprite;
    return bmp_sprite->data + (bmp_sprite->height - y - 1) * ((bmp_sprite->width + 3) & ~3);
}

static unsigned char bmp_transparent_color(const sprite_t* sprite)
{
  const unsigned char* sprite_ptr = sprite_first_stride(sprite);
  return sprite_ptr[0];
}

static void bmp_free(sprite_t* sprite)
{
    bmp_sprite_t* bmp_sprite = (bmp_sprite_t*) sprite;
    if(bmp_sprite)
    {
        if(bmp_sprite->data)
        {
            free(bmp_sprite->data);
        }
        free(bmp_sprite);
        bmp_sprite = NULL;
    }
}

sprite_t* create_bmp_sprite(const char *filename)
{
    bmp_sprite_t* result = NULL;
    FILE *file = fopen(filename, "rb");
    if (file) {
      BmpFileHeader file_header;

      fread(&file_header, sizeof(file_header), 1, file);
      if(file_header.bfType[0] == 'B' && file_header.bfType[1] == 'M')
      {
        BmpImageHeader image_header;
        fread(&image_header, sizeof(image_header), 1, file);
        if(image_header.biSize == 40 &&
           image_header.biCompression == 0 &&
           (image_header.biBitCount == 8 || image_header.biBitCount == 4))
        {
          result = (bmp_sprite_t*) malloc(sizeof(bmp_sprite_t));
          if(result != NULL) {
            unsigned int rounded_width = (image_header.biWidth + 3) & ~3;
            unsigned int rounded_height = image_header.biHeight;
            unsigned int raw_image_size = rounded_width * rounded_height;
            int i;

            result->width = image_header.biWidth;
            result->height = image_header.biHeight;
            result->colorCount = image_header.biClrUsed;

            fread(&(result->palette), sizeof(result->palette.colors[0]), result->colorCount, file);
            for(i = 0;i< result->colorCount; ++i)
            {
              result->palette.colors[i].red >>= 2;
              result->palette.colors[i].blue >>= 2;
              result->palette.colors[i].green >>= 2;
            }

            result->data = (unsigned char*) malloc(raw_image_size);
            if(result->data) {
              if(image_header.biBitCount == 8)
              {
                unsigned int read_result = fread(result->data, 1, raw_image_size, file);
              }
              else {

                unsigned int src_rounded_width = ((image_header.biWidth+1) / 2 + 3) & ~3;
                unsigned int src_size = result->height * src_rounded_width;
                unsigned char* raw_data = (unsigned char*) malloc(src_size);
                if(raw_data)
                {
                  int y = 0;
                  int x = 0;
                  int dpos = 0;
                  int spos = 0;

                  fread(raw_data, 1, src_size, file);
                  for(y = 0; y < result->height; ++y)
                  {
                    for(x = 0; x < (result->width+1) / 2; ++x)
                    {
                      result->data[dpos + 2*x] = raw_data[spos + x] >> 4;
                      result->data[dpos + 2*x + 1] = raw_data[spos + x] & 0x0F;
                    }
                    dpos += rounded_width;
                    spos += src_rounded_width;
                  }
                  free(raw_data);
                }
              }
            }
            else {
              free(result);
              result = NULL;
            }
          }
        }
      }
      fclose(file);
    }

    if(result != NULL)
    {
        result->sprite.width = bmp_width;
        result->sprite.height = bmp_height;
        result->sprite.color_count = bmp_color_count;
        result->sprite.palette = bmp_palette;
        result->sprite.first_stride = bmp_first_stride;
        result->sprite.next_stride = bmp_next_stride;
        result->sprite.get_stride = bmp_get_stride;
        result->sprite.transparent_color = bmp_transparent_color;
        result->sprite.free = bmp_free; 
    }

    return result == NULL ? NULL : &(result->sprite);
}
