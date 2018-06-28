/*
 * STM32ISP - CLI tool for ST STM32F10x In-System Programming
 *
 *  	copyright (c) 2011 BA5AG (ba5ag.kai@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32isp.h"
#include "hexfile.h"
#include "memmap.h"
#include "term.h"

static MemMap mp;

int stm32isp_init(const char *port, unsigned int baud, unsigned int bootp, unsigned int reset)
{
    int err;

    err = stm32Init(port, baud, bootp, reset);
    return err;
}

int stm32isp_load(const char *filename)
{
    int err =STM_ERR;

    stm32CmdGet();
    stm32GetID();

    if (readHexFile(filename, &mp))
    {
        stm32Erase();
	stm32Write(&mp);
	stm32Verify(&mp);
	stm32Go(0x08000000);
	err = STM_OK;
	printf("STM32isp load finished.\n");
    }
    else
    {
	printf("Can not open file %s.\n", filename);
    }

    return err;
}

void stm32isp_exit(void)
{
    stm32Close();
    mmapDelete(&mp);
    return ;
}

