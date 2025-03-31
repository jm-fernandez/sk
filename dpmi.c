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
#include "dpmi.h"
#include <stddef.h>

uint16_t dpmi_alloc_block(unsigned int len, uint16_t *sel)
{
	const unsigned int par = (len + 15) >> 4UL;
	if (len >= 0xFF00UL)
	{
    	return 0;
	}
	return dpmi_alloc(par, sel);
}

void dpmi_real_int(int inum, struct dpmi_real_regs *regs)
{
	__asm {
		mov eax, 0x0300
		mov edi, regs
		mov ebx, inum
		xor ecx, ecx
		int 0x31
	}
}

void * dpmi_mmap(uint32_t phys_addr, unsigned int size)
{
  void* result = NULL;

  _asm
  {
    mov ebx, phys_addr
    mov cx, bx
    shr ebx, 10h

    mov esi, size
    mov di, si
    shr esi, 10h

    mov ax, 0800h
    int 31h

    jc error

    shl ebx, 16
    mov bx, cx
    mov result, ebx
  error:
  }
  return result;
}


// void *dpmi_mmap(uint32_t phys_addr, unsigned int size)
// {
// 	uint16_t mem_high, mem_low;
// 	uint16_t phys_high = phys_addr >> 16;
// 	uint16_t phys_low = phys_addr & 0xffff;
// 	uint16_t size_high = size >> 16;
// 	uint16_t size_low = size & 0xffff;
// 	unsigned int err, res = 0;

// 	__asm {
// 		mov eax, 0x800
// 		mov bx, phys_high
// 		mov cx, phys_low
// 		mov si, size_high
// 		mov di, size_low
// 		int 0x31
// 		add res, 1
// 		mov err, eax
// 		mov mem_high, bx
// 		mov mem_low, cx
// 	}

// 	if(res == 2) {
// 		return 0;
// 	}
// 	return (void*)(((uint32_t)mem_high << 16) | ((uint32_t)mem_low));
// }

void dpmi_munmap(void *addr)
{
	uint16_t mem_high = (uint32_t)addr >> 16;
	uint16_t mem_low = (uint16_t)addr;

	__asm {
		mov eax, 0x801
		mov bx, mem_high
		mov cx, mem_low
		int 0x31
	}
}

dmpi_real_mode_address dpmi_alloc_cbk(void *fn, dpmi_real_regs *regs)
{
  dmpi_real_mode_address result = {0U};
  _asm
  {
        mov esi, fn
        mov edi, regs

        mov ax, cs
        mov ds, ax

        push ds
        pop es

		mov	ax, 0303h
        int 31h
        jc error

        lea esi, result
        mov WORD PTR[esi], cx
        mov WORD PTR[esi + 2], dx
error:
  }
  return result;
}

void dpmi_free_cbk( dmpi_real_mode_address real_mode_callback)
{
  _asm
  {
        lea esi, real_mode_callback
        mov cx, WORD PTR [esi]
        mov dx, WORD PTR [esi + 2]
        mov ax, 0304h
        int 31h
  }
}

bool dpmi_int_hndlr_set(unsigned char int_number, dmpi_real_mode_address real_addr)
{
  bool result = false;
  _asm
  {
        lea esi, real_addr
        mov cx, WORD PTR [esi]
        mov dx, WORD PTR [esi + 2]
        mov bl, int_number
        mov ax, 0201h
        int 31h
        jc  error
        inc result
    error:
  }
  return result;
}

dmpi_real_mode_address
dpmi_int_hndlr_get(unsigned char int_number)
{
  dmpi_real_mode_address result = {0U};
  _asm
  {
        mov bl, int_number
        mov ax, 0200h
        int 31h
        jc error
        lea esi, result
        mov WORD PTR[esi], cx
        mov WORD PTR[esi + 2], dx
    error:
  }
  return result;
}

