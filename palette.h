#ifndef PALETTE_H_
#define PALETTE_H_

typedef struct palette_entry_
{
  unsigned char blue;
  unsigned char green;
  unsigned char red;
  unsigned char alpha;
}palette_entry_t;

typedef struct palette_
{
  palette_entry_t colors[256];
} palette_t;

static inline void palette_invert(palette_t* palette)
{
  int i = 0;
  for(; i < 256; ++i)
  {
    palette->colors[i].red = 255 - palette->colors[i].red;
    palette->colors[i].blue = 255 - palette->colors[i].blue;
    palette->colors[i].green = 255 - palette->colors[i].green;
  }
}

#endif	/* SVGA_H_ */
