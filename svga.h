#ifndef SVGA_H_
#define SVGA_H_

#include <stdbool.h>

#include "palette.h"

#pragma pack(push, 1)
typedef struct VBE_PMInfoBlock {
  unsigned int Signature;        /*  PM Info Block Signature */
  unsigned short int EntryPoint; /*  Offset of PM entry point within BIOS */
  unsigned short int
      PMInitialize; /*  Offset of PM initialization entry point */
  unsigned short int
      BIOSDataSel;             /*  Selector to BIOS data area emulation block */
  unsigned short int A0000Sel; /*  Selector to access A0000h physical mem */
  unsigned short int B0000Sel; /*  Selector to access B0000h physical mem */
  unsigned short int B8000Sel; /*  Selector to access B8000h physical mem */
  unsigned short int CodeSegSel; /*  Selector to access code segment as data */
  unsigned char InProtectMode;   /*  Set to 1 when in protected mode */
  unsigned char Checksum;        /*  Checksum byte for structure */
} VBE_PMInfoBlock;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ModeInfoBlock_ {
  unsigned short ModeAttributes;
  unsigned char WinAAttributes;
  unsigned char WinBAttributes;
  unsigned short WinGranularity;
  unsigned short WinSize;
  unsigned short WinASegment;
  unsigned short WinBSegment;
  void (*WinFuncPtr)(void);
  unsigned short BytesPerScanLine;
  unsigned short XResolution;
  unsigned short YResolution;
  unsigned char XCharSize;
  unsigned char YCharSize;
  unsigned char NumberOfPlanes;
  unsigned char BitsPerPixel;
  unsigned char NumberOfBanks;
  unsigned char MemoryModel;
  unsigned char BankSize;
  unsigned char NumberOfImagePages;
  unsigned char res1;
  unsigned char RedMaskSize;
  unsigned char RedFieldPosition;
  unsigned char GreenMaskSize;
  unsigned char GreenFieldPosition;
  unsigned char BlueMaskSize;
  unsigned char BlueFieldPosition;
  unsigned char RsvdMaskSize;
  unsigned char RsvdFieldPosition;
  unsigned char DirectColorModeInfo;
  /* VESA 2.0 */
  void* PhysBasePtr;
  unsigned int OffScreenMemOffset;
  unsigned short OffScreenMemSize;
  /* VESA 3.0 */
  unsigned short LinBytesPerScanLine;
  unsigned char BnkNumberOfImagePages;
  unsigned char LinNumberOfImagePages;
  unsigned char LinRedMaskSize;
  unsigned char LinRedFieldPosition;
  unsigned char LinGreenMaskSize;
  unsigned char LinGreenFieldPositiondb;
  unsigned char LinBlueMaskSize;
  unsigned char LinBlueFieldPosition;
  unsigned char LinRsvdMaskSize;
  unsigned char LinRsvdFieldPosition;
  unsigned int MaxPixelClock;  
  unsigned char res2[189];      
} ModeInfoBlock;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct SVGAMode_ {
  unsigned short mode;
  unsigned short XResolution;
  unsigned short YResolution;
  unsigned char BitsPerPixel;
  unsigned short ModeAttributes;
  unsigned char* BasePtr;
  unsigned int ScanlineLength;
  unsigned char NumberOfImagePages;
  unsigned char CurrentPage;
}SVGAMode;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct VbeInfoBlock_ {
  char VESASignature[4];
  unsigned short VESAVersion;
  char *OEMStringPtr;
  unsigned long Capabilities;
  unsigned short *VideoModePtr;
  unsigned short TotalMemory;
  unsigned short OemSoftwareRev;
  char *OemVendorNamePtr;
  char *OemProductNamePtr;
  char *OemProductRevPtr;
  char reserved[222];
  char OemData[256];
} VbeInfoBlock;
#pragma pack(pop)

// flags for ModeAttributes
#define LINEAR_FRAME_BUFFER_AVAILABLE (0x80)

typedef bool (*check_video_mode_t)(SVGAMode* info, void* user_data);

void set_video_mode(int mode);
bool set_super_vga_video_mode(int mode);
bool get_video_mode_info(unsigned int mode, SVGAMode* info);
bool find_video_mode(SVGAMode* info, check_video_mode_t check_video_mode, void* user_data);
void set_palette_format_8_bits();
bool set_palette_data(const palette_t* palette);
void set_display_start(SVGAMode* video_info, unsigned int scanline_offset, unsigned int scanline, bool retrace);
unsigned int set_logical_scanline_length(unsigned int scanline_length);

#endif	/* SVGA_H_ */
