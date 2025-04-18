/*
auplay - audio player for DOS.
Copyright (C) 2019-2022  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dos.h>
#include <conio.h>
#include "audio.h"
#include "audrv.h"
#include "dpmi.h"
#include "dma.h"
#include "intr.h"

#define BUFSIZE			16384

#define REG_MIXPORT		(base_port + 0x4)
#define REG_MIXDATA		(base_port + 0x5)
#define REG_RESET		(base_port + 0x6)
#define REG_RDATA		(base_port + 0xa)
#define REG_WDATA		(base_port + 0xc)
#define REG_WSTAT		(base_port + 0xc)
#define REG_RSTAT		(base_port + 0xe)
#define REG_INTACK		(base_port + 0xe)
#define REG_INT16ACK	(base_port + 0xf)

#define WSTAT_BUSY		0x80
#define RSTAT_RDY		0x80

#define DSP_RATE			0x40
#define DSP4_OUT_RATE		0x41
#define DSP4_IN_RATE		0x42
#define DSP_BLOCKSZ			0x48
#define DSP_GET_VER			0xe1

/* start DMA playback/recording. combine with fifo/auto/input flags */
#define DSP4_START_DMA8		0xc0
#define DSP4_START_DMA16	0xb0
#define DSP4_FIFO			0x02
#define DSP4_AUTO			0x04
#define DSP4_INPUT			0x08

/* transfer mode commands */
#define DSP4_MODE_SIGNED	0x10
#define DSP4_MODE_STEREO	0x20

/* immediately pause/continue */
#define DSP_PAUSE_DMA8		0xd0
#define DSP_ENABLE_OUTPUT	0xd1
#define DSP_DISABLE_OUTPUT	0xd3
#define DSP_CONT_DMA8		0xd4
#define DSP_PAUSE_DMA16		0xd5
#define DSP_CONT_DMA16		0xd6

/* end the playback at the end of the current buffer */
#define DSP_END_DMA16		0xd9
#define DSP_END_DMA8		0xda

/* mixer registers */
#define MIX_MASTER			0x02
#define MIX_VOICE			0x0a
#define MIX_SBPRO_VOICE		0x04
#define MIX_SBPRO_MASTER	0x22
#define MIX_SB16_MASTER_L	0x30
#define MIX_SB16_MASTER_R	0x31
#define MIX_SB16_VOICE_L	0x32
#define MIX_SB16_VOICE_R	0x33

#define MIX_SBPRO_STEREO	0x0e
#define MIX_SB16_IRQ_SEL	0x80
#define MIX_SB16_DMA_SEL	0x81
#define MIX_SB16_INTSTAT	0x82

#define INTSTAT_DMA8		0x01
#define INTSTAT_DMA16		0x02
#define INTSTAT_MPU401		0x04

#define VER_MAJOR(x)	((x) >> 8)
#define VER_MINOR(x)	((x) & 0xff)

/* delay for about 1us */
#define iodelay()	outp(0x80, 0)


static int reset_dsp(void);
static void *sb_buffer(int *size);
static void start(int rate, int bits, int nchan);
static void pause(void);
static void cont(void);
static void stop(void);
static int isplaying(void);
static void setvolume(int ctl, int vol);
static int getvolume(int ctl);

static void set_sbpro_stereo(void);
static int start_dsp4(int rate, int bits, unsigned int mode, int num_samples);
static int start_dsp(int rate, int nchan, int num_samples);
static void INTERRUPT intr_handler();
static void write_dsp(unsigned char val);
static unsigned char read_dsp(void);
static void write_mix(int reg, unsigned char val);
static unsigned char read_mix(int reg);
static int get_dsp_version(void);
static int dsp4_detect_irq(void);
static int dsp4_detect_dma(void);
static const char *sbname(int ver);


static const struct audrv sbdrv = {
	start,
	pause,
	cont,
	stop,
	setvolume,
	getvolume,
	isplaying
};


static int base_port;
static int irq, dma_chan, dma16_chan, dsp_ver;

static void *buffer[2];
static volatile int wrbuf;

static volatile int playing;
static int cur_bits, auto_init, high_speed;

static void (INTERRUPT *prev_intr_handler)();


int sb_detect(struct audrv *drv)
{
	int i;
	char *env;

	if((env = getenv("BLASTER"))) {
		dma16_chan = -1;
		if(sscanf(env, "A%x I%d D%d H%d", &base_port, &irq, &dma_chan, &dma16_chan) >= 3) {
			if(reset_dsp() == 0) {
				dsp_ver = get_dsp_version();
				*drv = sbdrv;
				return 1;
			}
		} else {
			printf("sb_detect: malformed BLASTER environment variable. Fallback to probing.\n");
		}
	}

	for(i=0; i<6; i++) {
		base_port = 0x200 + ((i + 1) << 4);
		if(reset_dsp() == 0) {
			dsp_ver = get_dsp_version();

			if(dsp_ver >= 0x400) {
				if(dsp4_detect_irq() == -1) {
					printf("sb_detect: failed to configure IRQ\n");
					return 0;
				}
				if(dsp4_detect_dma() == -1) {
					printf("sb_detect: failed to configure DMA\n");
					return 0;
				}

			} else {
				/* XXX for old sound blasters, hard-code to IRQ 5 DMA 1 for now */
				irq = 5;
				dma_chan = 1;
				dma16_chan = -1;

				printf("sb_detect: old sound blaster dsp. assuming: irq 5, dma 1\n");
			}
			*drv = sbdrv;

			return 1;
		}
	}

	return 0;
}

static int reset_dsp(void)
{
	int i;

	outp(REG_RESET, 1);
	for(i=0; i<3; i++) iodelay();
	outp(REG_RESET, 0);

	for(i=0; i<128; i++) {
		if(inp(REG_RSTAT) & RSTAT_RDY) {
			if(inp(REG_RDATA) == 0xaa) {
				return 0;
			}
		}
	}

	return -1;
}

static void start(int rate, int bits, int nchan)
{
	uint16_t seg, pmsel;
	uint32_t addr, next64k;
	int size, mode, num_samples, dsp_num_samples;

	if(!buffer[1]) {
		/* allocate a buffer in low memory that doesn't cross 64k boundaries */
		if(!(seg = dpmi_alloc(BUFSIZE * 2 / 16, &pmsel))) {
			fprintf(stderr, "SB start: failed to allocate DMA buffer\n");
			return;
		}

		addr = (uint32_t)seg << 4;
		next64k = (addr + 0x10000) & 0xffff0000;
		if(next64k - addr < BUFSIZE) {
			addr = next64k;
		}

		buffer[0] = (void*)addr;
		buffer[1] = (void*)(addr + BUFSIZE / 2);
		wrbuf = 0;
	}


	wrbuf = 0;
	if(!(size = audio_callback(buffer[wrbuf], BUFSIZE))) {
		return;
	}
	addr = (uint32_t)buffer[wrbuf];
	num_samples = bits == 8 ? size : size / 2;

	if(size < BUFSIZE) {
		auto_init = 0;	/* single transfer mode */
		dsp_num_samples = num_samples;
	} else {
		/* Auto-init playback mode:
		 * fill the whole buffer with data, program the DMA for BUFSIZE transfer,
		 * and program the DSP for BUFSIZE/2 transfer. We'll get an interrupt in the
		 * middle, while the DSP uses the upper half, and we'll refill the bottom half.
		 * Then continue ping-ponging the two halves of the buffer until we run out of
		 * data.
		 */
		auto_init = 1;
		dsp_num_samples = num_samples / 2;
	}

	_disable();
	if(!prev_intr_handler) {
		prev_intr_handler = _dos_getvect(IRQ_TO_INTR(irq));
	}
	_dos_setvect(IRQ_TO_INTR(irq), intr_handler);
	_enable();

	unmask_irq(irq);

	cur_bits = bits;

	write_dsp(DSP_ENABLE_OUTPUT);
	dma_out(cur_bits == 8 ? dma_chan : dma16_chan, addr, BUFSIZE, DMA_SINGLE | auto_init ? DMA_AUTO : 0);

	if(dsp_ver >= 0x400) {
		int mode = bits == 8 ? 0 : DSP4_MODE_SIGNED;
		if(nchan > 1) {
			mode |= DSP4_MODE_STEREO;
		}
		start_dsp4(rate, cur_bits, mode, dsp_num_samples);
	} else {
		if(nchan > 1 && dsp_ver >= 0x300) {
			set_sbpro_stereo();
		}
		start_dsp(rate, cur_bits, dsp_num_samples);
	}
	playing = 1;
}

static void pause(void)
{
	write_dsp(cur_bits == 8 ? DSP_PAUSE_DMA8 : DSP_PAUSE_DMA16);
}

static void cont(void)
{
	write_dsp(cur_bits == 8 ? DSP_CONT_DMA8 : DSP_CONT_DMA16);
}

static void stop(void)
{
	write_dsp(DSP_DISABLE_OUTPUT);

	mask_irq(irq);
	/* TODO: don't _enable, restore state */
	_disable();
	_dos_setvect(IRQ_TO_INTR(irq), prev_intr_handler);
	_enable();

	write_dsp(cur_bits == 8 ? DSP_END_DMA8 : DSP_END_DMA16);
	playing = 0;
}

static int isplaying(void)
{
	return playing;
}

static void setvolume(int ctl, int vol)
{
	unsigned char val;

	if(VER_MAJOR(dsp_ver) >= 4) {
		/* DSP 4.x - SB16 */
		val = vol & 0xf8;
		if(ctl == AUDIO_PCM) {
			write_mix(MIX_SB16_VOICE_L, val);
			write_mix(MIX_SB16_VOICE_R, val);
		} else {
			write_mix(MIX_SB16_MASTER_L, val);
			write_mix(MIX_SB16_MASTER_R, val);
		}

	} else if(VER_MAJOR(dsp_ver) >= 3) {
		/* DSP 3.x - SBPro */
		val = vol & 0xe0;
		val |= val >> 4;

		if(ctl == AUDIO_PCM) {
			write_mix(MIX_SBPRO_VOICE, val);
		} else {
			write_mix(MIX_SBPRO_MASTER, val);
		}

	} else {
		/* DSP 2.x - SB 2.0 */
		if(ctl == AUDIO_PCM) {
			val = (vol >> 5) & 6;
			write_mix(MIX_VOICE, val);
		} else {
			val = (vol >> 4) & 0xe;
			write_mix(MIX_MASTER, val);
		}
	}
}

static int getvolume(int ctl)
{
	int left, right;
	unsigned char val;

	if(dsp_ver >= 0x400) {
		/* DSP 4.x - SB16 */
		int lreg, rreg;
		if(ctl == AUDIO_PCM) {
			lreg = MIX_SB16_VOICE_L;
			rreg = MIX_SB16_VOICE_R;
		} else {	/* MASTER or DEFAULT */
			lreg = MIX_SB16_MASTER_L;
			rreg = MIX_SB16_MASTER_R;
		}

		val = read_mix(lreg);
		left = (val & 0xf8) | (val >> 5);
		val = read_mix(rreg);
		right = (val & 0xf8) | (val >> 5);

	} else if(dsp_ver >= 0x300) {
		/* DSP 3.x - SBPro */
		val = read_mix(ctl == AUDIO_PCM ? MIX_SBPRO_VOICE : MIX_SBPRO_MASTER);

		/* left is top 3 bits, duplicate twice(-ish) */
		left = (val & 0xe0) | ((val >> 3) & 0x1c) | (val >> 6);
		/* right is top 3 bits of the lower nibble */
		right = (val << 4) | ((val << 1) & 0x1c) | ((val >> 2) & 3);

	} else {
		int volume;

		/* DSP 2.x - SB 2.0 */
		if(ctl == AUDIO_PCM) {
			/* voice is in bits 1 and 2 */
			val = read_mix(MIX_VOICE);
			volume = (val << 5) | ((val << 3) & 0x30) | ((val << 1) & 0xc) | ((val >> 1) & 3);
		} else {
			/* master is in the 3 top bits of the lower nibble */
			val = read_mix(MIX_MASTER);
			volume = (val << 4) | ((val << 1) & 0x1c) | ((val >> 2) & 3);
		}
		return volume;
	}

	return (left + right) >> 1;
}

static void set_sbpro_stereo(void)
{
}

static int start_dsp4(int rate, int bits, unsigned int mode, int num_samples)
{
	unsigned char cmd = bits == 8 ? DSP4_START_DMA8 : DSP4_START_DMA16;
	if(auto_init) {
		cmd |= DSP4_AUTO | DSP4_FIFO;
	}

	/* set output rate */
	write_dsp(DSP4_OUT_RATE);
	write_dsp(rate >> 8);
	write_dsp(rate & 0xff);

	/* program the DSP to start the DMA transfer */
	write_dsp(cmd);
	write_dsp(mode);
	num_samples--;
	write_dsp(num_samples & 0xff);
	write_dsp((num_samples >> 8) & 0xff);

	return 0;
}

static int start_dsp(int rate, int nchan, int num_samples)
{
	/* set time constant */
	int tcon = 256 - 1000000 / rate;
	write_dsp(DSP_RATE);
	write_dsp(tcon);

	if(nchan > 1) {
		/* stereo */
		if(dsp_ver < 0x300) return -1;	/* need a sb pro for stereo */

		set_sbpro_stereo();
		high_speed = rate >= 11025;
	} else {
		/* mono */
		high_speed = rate >= 23000;
	}

	/* program the DSP to start the DMA transfer */
	return -1;	/* TODO */
}


static void INTERRUPT intr_handler()
{
	int size;
	void *bptr = buffer[wrbuf];
	uint32_t addr = (uint32_t)bptr;

	wrbuf ^= 1;

	/* ask for more data */
	if(!(size = audio_callback(bptr, BUFSIZE / 2))) {
		stop();
	}

	/* acknowledge the interrupt */
	if(cur_bits == 8) {
		inp(REG_INTACK);
	} else {
		unsigned char istat = read_mix(MIX_SB16_INTSTAT);
		if(istat & INTSTAT_DMA16) {
			inp(REG_INT16ACK);
		}
	}

	if(irq > 7) {
		outp(PIC2_CMD, OCW2_EOI);
	}
	outp(PIC1_CMD, OCW2_EOI);
}

static void write_dsp(unsigned char val)
{
	while(inp(REG_WSTAT) & WSTAT_BUSY);
	outp(REG_WDATA, val);
}

static unsigned char read_dsp(void)
{
	while((inp(REG_RSTAT) & RSTAT_RDY) == 0);
	return inp(REG_RDATA);
}

static void write_mix(int reg, unsigned char val)
{
	outp(REG_MIXPORT, reg);
	outp(REG_MIXDATA, val);
}

static unsigned char read_mix(int reg)
{
	outp(REG_MIXPORT, reg);
	return inp(REG_MIXDATA);
}

static int get_dsp_version(void)
{
	int major, minor;

	write_dsp(DSP_GET_VER);
	major = read_dsp();
	minor = read_dsp();

	return (major << 8) | minor;
}

static int dsp4_detect_irq(void)
{
	int i, irqsel;
	static int irqtab[] = {2, 5, 7, 10};

	irq = 0;
	irqsel = read_mix(MIX_SB16_IRQ_SEL);
	for(i=0; i<4; i++) {
		if(irqsel & (1 << i)) {
			irq = irqtab[i];
			break;
		}
	}
	if(!irq) {
		/* try to force IRQ 5 */
		write_mix(MIX_SB16_IRQ_SEL, 2);	/* bit1 selects irq 5 */

		/* re-read to verify */
		irqsel = read_mix(MIX_SB16_IRQ_SEL);
		if(irqsel != 2) {
			return -1;
		}
		irq = 5;
	}

	return irq;
}

static int dsp4_detect_dma(void)
{
	int i, dmasel, tmp;
	static int dmatab[] = {0, 1, -1, 3, -1, 5, 6, 7};

	dma_chan = -1;
	dma16_chan = -1;
	dmasel = read_mix(MIX_SB16_DMA_SEL);
	for(i=0; i<4; i++) {
		if(dmasel & (1 << i)) {
			dma_chan = dmatab[i];
			break;
		}
	}
	for(i=5; i<8; i++) {
		if(dmasel & (1 << i)) {
			dma16_chan = dmatab[i];
			break;
		}
	}
	if(dma_chan == -1) {
		/* try to force DMA 1 */
		dmasel |= 2;
	}
	if(dma16_chan == -1) {
		/* try to force 16bit DMA 5 */
		dmasel |= 0x20;
	}

	if(dma_chan == -1 || dma16_chan == -1) {
		write_mix(MIX_SB16_DMA_SEL, dmasel);

		/* re-read to verify */
		tmp = read_mix(MIX_SB16_DMA_SEL);
		if(tmp != dmasel) {
			return -1;
		}
		dma_chan = 1;
		dma16_chan = 5;
	}

	return dma_chan;
}



#define V(maj, min)	(((maj) << 8) | (min))

static const char *sbname(int ver)
{
	int major = VER_MAJOR(ver);
	int minor = VER_MINOR(ver);

	switch(major) {
	case 1:
		if(minor == 5) {
			return "Sound Blaster 1.5";
		}
		return "Sound Blaster 1.0";

	case 2:
		if(minor == 1 || minor == 2) {
			return "Sound Blaster 2.0";
		}
		break;

	case 3:
		switch(minor) {
		case 0:
			return "Sound Blaster Pro";
		case 1:
		case 2:
			return "Sound Blaster Pro 2";
		case 5:
			return "Gallant SC-6000";
		default:
			break;
		}
		break;

	case 4:
		switch(minor) {
		case 4:
		case 5:
			return "Sound Blaster 16";
		case 11:
			return "Sound Blaster 16 SCSI-2";
		case 12:
			return "Sound Blaster AWE 32";
		case 13:
			return "Sound Blaster ViBRA16C";
		case 16:
			return "Sound Blaster AWE 64";
		default:
			break;
		}
		break;
	}

	return "Unknown Sound Blaster";
}
