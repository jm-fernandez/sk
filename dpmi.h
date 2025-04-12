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
#ifndef DPMI_H_
#define DPMI_H_

#include <stdint.h>
#include <stdbool.h>

#define DPMI_INVALID_DESCRIPTOR ((unsigned short)-1)

typedef struct dpmi_real_regs {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t reserved;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint16_t flags;
	uint16_t es; 
	uint16_t ds;
	uint16_t fs;
	uint16_t gs;
	uint16_t ip;
	uint16_t cs;
	uint16_t sp;
	uint16_t ss;
} dpmi_real_regs;

typedef struct dmpi_real_mode_address_ {
  uint16_t sgmnt;
  uint16_t offst;
} dmpi_real_mode_address;

static void* dpmi_pointer_from_segment_base(uint16_t segment_base)
{
	return (void*)((intptr_t)segment_base << 4U);
}

uint16_t dpmi_alloc_block(unsigned int len, uint16_t *sel);

uint16_t dpmi_alloc(unsigned int par, uint16_t *sel);
void dpmi_free(uint16_t sel);

dmpi_real_mode_address dpmi_alloc_cbk(void *fn, dpmi_real_regs *regs);
void dpmi_free_cbk( dmpi_real_mode_address real_mode_callback);

#pragma aux dpmi_alloc = \
		"mov eax, 0x100" \
		"int 0x31" \
		"mov [edi], dx" \
		value[ax] parm[ebx][edi];

#pragma aux dpmi_free = \
		"mov eax, 0x101" \
		"int 0x31" \
		parm[dx];

void dpmi_real_int(int inum, struct dpmi_real_regs *regs);

void *dpmi_mmap(void* phys_addr, unsigned int size);
void dpmi_unmap(void *addr);

unsigned short dpmi_allocate_ldt_descriptor();
unsigned short dpmi_create_ldt_descriptor(void* address, unsigned int limit);
void dpmi_free_ldt_descriptor(unsigned short descriptor);
bool dpmi_set_ldt_base_address(unsigned short descriptor, void* address);
bool dpmi_set_ldt_limit(unsigned short descriptor, unsigned int limit);
bool dpmi_set_ldt_rights(unsigned short descriptor, unsigned char rights, unsigned char extended_rights);


dmpi_real_mode_address dpmi_int_hndlr_get(unsigned char int_number);
bool dpmi_int_hndlr_set(unsigned char int_number, dmpi_real_mode_address real_addr);

#endif	/* DPMI_H_ */
