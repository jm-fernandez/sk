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
#include "aufile.h"
#include <stdint.h>
#include <string.h>

struct format {
	uint16_t fmt;
	uint16_t nchan;
	uint32_t rate;
	uint16_t avgbaud;
	uint16_t block_align;
	uint16_t sample_bytes;
};

#define FOURCC(a, b, c, d) \
	((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

enum {
	ID_RIFF	= FOURCC('R', 'I', 'F', 'F'),
	ID_WAVE	= FOURCC('W', 'A', 'V', 'E'),
	ID_FMT = FOURCC('f', 'm', 't', ' '),
	ID_DATA = FOURCC('d', 'a', 't', 'a')
};

static void close_wav(struct au_file *au);
static void reset_wav(struct au_file *au);
static int read_wav(struct au_file *au, void *buf, int size);
static int read_uint32(uint32_t *res, FILE *fp);
static int read_format(struct format *fmt, int fmtsize, FILE *fp);


int au_open_wav(struct au_file *au, FILE* fp)
{
	uint32_t id, len;
	struct format fmt;

	if(read_uint32(&id, fp) == -1 || id != ID_RIFF)
	{
		return -1;
	}
	fseek(fp, 4, SEEK_CUR);
	if(read_uint32(&id, fp) == -1 || id != ID_WAVE)
	{
		return -1;
	}
	if(read_uint32(&id, fp) == -1 || id != ID_FMT)
	{
		return -1;
	}
	if(read_uint32(&len, fp) == -1)
	{
		return -1;
	}
	if(read_format(&fmt, len, fp) == -1)
	{
		return -1;
	}
	if(read_uint32(&id, fp) == -1 || id != ID_DATA)
	{
		return -1;
	}
	if(read_uint32(&len, fp) == -1)
	{
		return -1;
	}

	au->rate = fmt.rate;
	au->bits = fmt.sample_bytes * 8;
	au->chan = fmt.nchan;

	au->data.size = len;
	au->data.pos = 0;
	au->data.buffer = (uint8_t*) malloc(len * sizeof(uint8_t));
	if(au->data.buffer)
	{
		if(fread(au->data.buffer, 1, len, fp) < len)
		{
			free(au->data.buffer);
			au->data.buffer = NULL;
			return -1;
		}
	}

	return 0;
}

void au_close_wav(struct au_file *au)
{
	free(au->data.buffer);
	au->data.buffer = NULL;
}

void au_reset_wav(struct au_file *au)
{
	au->data.pos = 0;
}

int au_read_wav(struct au_file *au, void *buf, int size)
{
	const int bytes_left = au->data.size - au->data.pos;
	if(size > bytes_left) {
		size = bytes_left;
	}

	if(size > 0)
	{
		memcpy(buf, au->data.buffer + au->data.pos, size);
		au->data.pos += size;
	}

	return size;
}

static int read_uint32(uint32_t *res, FILE *fp)
{
	if(fread(res, 4, 1, fp) < 1) {
		return -1;
	}
	return 0;
}

static int read_format(struct format *fmt, int fmtsize, FILE *fp)
{
	if(fread(fmt, 1, fmtsize, fp) < fmtsize) {
		return -1;
	}
	return 0;
}
