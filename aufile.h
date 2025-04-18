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
#ifndef AUFILE_H_
#define AUFILE_H_

#include <stdio.h>
#include <stdint.h>

struct playback_data {
	uint8_t* buffer;
	uint32_t size;
	uint32_t pos;
};

struct au_file {
	int rate;
	int bits;
	int chan;
	struct playback_data data;
};

struct au_file *au_open(const char *fname);
void au_close(struct au_file *au);

void au_reset(struct au_file *au);
int au_read(struct au_file *au, void *buf, int size);

#endif	/* AUFILE_H_ */
