/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 2002, 2003 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __VBE_H
#define __VBE_H

#include <dpmi.h>

/* OEM information */
#define OEM_VENDOR_STR "Andrea Mazzoleni"
#define OEM_PRODUCT_STR "AdvanceVBE32"
#define OEM_VERSION_NUM 0x0102
#define OEM_VERSION_STR "1.2 " __DATE__
#define OEM_STR OEM_VENDOR_STR " " OEM_PRODUCT_STR " " OEM_VERSION_STR

#define TSR_RUTACK1 0xAD17
#define TSR_RUTACK2 0x17BE

int vbe_init(const char* config);
void vbe_done(void);

void vbe_service(_go32_dpmi_registers* r);
void vga_service(_go32_dpmi_registers* r);

#endif
