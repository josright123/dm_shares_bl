/**
 * Copyright (c) 2023-2025 Davicom Semiconduct., Inc.
 * 20240411 Joseph CHANG
 */
#include <stdio.h>
#include <lwip/err.h>
#include "include/develop.h"
#include "dm9051_bl.h"
#include "dm9051_bl_conf.h"

static uint8_t lw_flag;

int dm9051_get_flags(void)
{
	return dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP);
}

void dm9051_update_flags(uint8_t nsr)
{
	if (nsr  & NSR_LINKST)
		dm9051_set_flags(lw_flag, DM9051_FLAG_LINK_UP);
	else
		dm9051_clear_flags(lw_flag, DM9051_FLAG_LINK_UP);
}

void dm9051_delay_in_core_process(uint16_t nms) //finally, dm9051_lw.c
{
	printf("dm9051_driver setup delay %u ms..\r\n", nms);
	vTaskDelay(nms*50); //dm9051_if.delay_ticks(nms*50); //from James' advice! to be determined with a reproduced test cases!!
}
