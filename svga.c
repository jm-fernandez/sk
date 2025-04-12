#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "svga.h"
#include "dpmi.h"

void set_video_mode(int mode) {
	dpmi_real_regs register_data_structure = {0};
	register_data_structure.eax = mode;
  dpmi_real_int(0x10, &register_data_structure);
}

bool set_super_vga_video_mode(int mode) {
	dpmi_real_regs register_data_structure = {0};
	register_data_structure.eax = 0x4F02;
	register_data_structure.ebx = mode;
  dpmi_real_int(0x10, &register_data_structure);
  return (((register_data_structure.eax >> 8) & 0xFF) == 0U);
}

bool get_video_mode_info(unsigned int mode, SVGAMode* info) {
  bool result = false;
  unsigned short selector;
  const uint16_t segment_base = dpmi_alloc_block(sizeof(ModeInfoBlock), &selector);
  if (segment_base)
  {
    dpmi_real_regs register_data_structure = {0};
    unsigned char* base_ptr = NULL;
    ModeInfoBlock *mode_info_block = dpmi_pointer_from_segment_base(segment_base);
    register_data_structure.eax = 0x4F01;
    register_data_structure.ecx = mode;
    register_data_structure.edi = 0;
    register_data_structure.es = segment_base;

    dpmi_real_int(0x10, &register_data_structure);

    info->mode = mode;
    info->XResolution = mode_info_block->XResolution;
    info->YResolution = mode_info_block->YResolution;
    info->BitsPerPixel = mode_info_block->BitsPerPixel;
    info->ModeAttributes = mode_info_block->ModeAttributes;
    info->PhysBasePtr = mode_info_block->PhysBasePtr;
    info->NumberOfImagePages = mode_info_block->NumberOfImagePages;

    result = true;
    dpmi_free(selector);
  }
  return result;
}

bool find_video_mode(SVGAMode* info, check_video_mode_t check_video_mode, void* user_data)
{
  bool result = false;
  unsigned short selector;
  const uint16_t segment_base = dpmi_alloc_block(sizeof(VbeInfoBlock), &selector);
  if (segment_base) {
    VbeInfoBlock *vbe_info_block = (VbeInfoBlock *) dpmi_pointer_from_segment_base(segment_base);
    dpmi_real_regs register_data_structure = {0};
    register_data_structure.eax = 0x4F00;
    register_data_structure.edi = 0;
    register_data_structure.es = segment_base;

    vbe_info_block->VESASignature[0] = 'V';
    vbe_info_block->VESASignature[1] = 'B';
    vbe_info_block->VESASignature[2] = 'E';
    vbe_info_block->VESASignature[3] = '2';

    dpmi_real_int(0x10, &register_data_structure);
    if ((vbe_info_block->VESAVersion >> 8) == 2) {
      unsigned short *mode_ptr = vbe_info_block->VideoModePtr;

      mode_ptr = (unsigned short *)((((unsigned int)mode_ptr >> 16UL) << 4UL) +
                                    ((unsigned int)mode_ptr & 0x1FF));

      while (*mode_ptr != (unsigned short)-1) {
        const unsigned short mode = *mode_ptr++;
        if((mode >= 0x100) && get_video_mode_info(mode, info) && check_video_mode(info, user_data)) {
          info->Capabilities = (unsigned short) vbe_info_block->Capabilities;
          result = true;
          break;
        }
      }
    }
    dpmi_free(selector);
  }
  return result;
}

bool set_palette_format(int bits)
{
    dpmi_real_regs register_data_structure = {0};

    register_data_structure.eax = 0x4F08;
    register_data_structure.ebx = bits * 0x100;
    dpmi_real_int(0x10, &register_data_structure);

    return (unsigned short) register_data_structure.eax == VESA_RESULT_OK
      && get_palette_format() == bits;
}

int get_palette_format()
{
  int result = -1;
  dpmi_real_regs register_data_structure = {0};

  register_data_structure.eax = 0x4F08;
  register_data_structure.ebx = 1;
  dpmi_real_int(0x10, &register_data_structure);

  if((unsigned short) register_data_structure.eax == VESA_RESULT_OK)
  {
    result = (unsigned char) (register_data_structure.ebx >> 8);
  }
  return result;
}

bool set_palette_data(const palette_t* palette) {
  bool result = false;
  unsigned short selector;
  const uint16_t segment_base = dpmi_alloc_block(256 * 4, &selector);
  if (segment_base) {
    unsigned char* color = (unsigned char*) dpmi_pointer_from_segment_base(segment_base);
    dpmi_real_regs register_data_structure = {0};
    int i = 0;

    for(; i < 256; ++i) {
      *color++ = palette->colors[i].blue;
      *color++ = palette->colors[i].green;
      *color++ = palette->colors[i].red;
      ++color;
    }

    register_data_structure.eax = 0x4F09;
    register_data_structure.ebx = 0;
    register_data_structure.edx = 0;
    register_data_structure.ecx = 256;
    register_data_structure.edi = 0;
    register_data_structure.es = segment_base;

    dpmi_real_int(0x10, &register_data_structure);
    dpmi_free(selector);
    result = true;
  }
  return result;
}

void set_display_start(SVGAMode* video_info, unsigned int scanline_offset, unsigned int scanline, bool retrace)
{
  dpmi_real_regs register_data_structure = {0};

  register_data_structure.eax = 0x4F07;
  register_data_structure.ebx = retrace ? 0x80 : 0x01;
  register_data_structure.ecx = scanline_offset;
  register_data_structure.edx = scanline;

  dpmi_real_int(0x10, &register_data_structure);
}


bool set_logical_scanline_length(unsigned int scanline_length, svga_scanline_info* info)
{
  bool result = false;
  dpmi_real_regs register_data_structure = {0};

  register_data_structure.eax = 0x4F06;
  register_data_structure.ecx = scanline_length;

  dpmi_real_int(0x10, &register_data_structure);

  info->bytes_per_scanline = register_data_structure.ebx & 0xFFFF;
  info->pixels_per_scanline = register_data_structure.ecx & 0xFFFF;
  info->total_scanlines = register_data_structure.edx & 0xFFFF;

  return (((register_data_structure.eax >> 8) & 0xFF) == 0U);
}

bool get_logical_scanline_length(svga_scanline_info* info)
{
  bool result = false;
  dpmi_real_regs register_data_structure = {0};

  register_data_structure.eax = 0x4F06;
  register_data_structure.ebx = 1;

  dpmi_real_int(0x10, &register_data_structure);

  info->bytes_per_scanline = register_data_structure.ebx & 0xFFFF;
  info->pixels_per_scanline = register_data_structure.ecx & 0xFFFF;
  info->total_scanlines = register_data_structure.edx & 0xFFFF;

  return (((register_data_structure.eax >> 8) & 0xFF) == 0U);
}
