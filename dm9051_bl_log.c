/**
 * Copyright (c) 2023-2025 Davicom Semiconduct., Inc.
 * 20240411 Joseph CHANG
 */
#include <stdio.h>
#include <lwip/err.h>
#include "include/develop.h"
#include "dm9051_bl.h"
#include "dm9051_bl_conf.h"

static struct spi_regcmd {
	uint8_t cmd;
	uint8_t data;
	uint8_t rdata;
} regcmd;

void dm9051_show_lock_arch(const char *project_name)
{
//	const char *test1 = "TESTED";

	bannerline_log();
//	printf("%s start: LWIP_TCPIP_CORE_LOCKING %d  /  LOCK_TCPIP_CORE()= %s\r\n", test1,
//			1, "sys_mutex_unlock(&lock_tcpip_core)");
//	printf("%s start: freeRTOS %d  / (TESTED GOOD, Must be sure that SPI lines are solid connection)\r\n", test1, 1);
	
//#if LWIP_TCPIP_CORE_LOCKING//#else//#endif
//#if freeRTOS//#endif
	bannerdash_log("[using lwip 3rdparty]");
	printf("%s start: LWIP_TCPIP_CORE_LOCKING %d  /  LOCK_TCPIP_CORE()= %s\r\n", project_name,
			LWIP_TCPIP_CORE_LOCKING, "(none)");
	printf("%s start: freeRTOS %d  / (TESTED GOOD, now, check \"freertos_tcpip_stack/port/lwipopts.h\")\r\n", project_name, /*freeRTOS*/ 0);
	
	bannerline_log();
}

void display_spi_info(hosal_spi_dev_t *pspi)
{
	blog_info("spi speed    %d\r\n", pspi->config.freq);
	blog_info("spi pin cs   IO%d\r\n", DM9051_SPI_CS);
	blog_info("spi pin clk  IO%d\r\n", pspi->config.pin_clk);
	
	blog_info("spi pin mosi IO%d\r\n", pspi->config.pin_mosi);
	blog_info("spi pin miso IO%d\r\n", pspi->config.pin_miso);
}

int master_main_loop = 1; //0;
int master_send_complete = 1;

void spi_master_cb(void *arg)
{
	regcmd.cmd = send_data[0];
	
	if (!(regcmd.cmd & DM_REG_W)) {
		regcmd.data = NULL_DONTCARE;
		regcmd.rdata = recv_data[1];
		if (master_main_loop == 0 || regcmd.rdata != 0xff) {
			if (regcmd.cmd == DM9051_PIDL || regcmd.cmd == DM9051_PIDH /*|| regcmd.cmd == 0x5C*/ ) {
					
				blog_info("master send is [%02x][..]\r\n",regcmd.cmd);
				blog_info("master recv is [..][%02x]\r\n",regcmd.rdata);
				master_main_loop = 1;
			}
		}
	}
	
	if (regcmd.cmd & DM_REG_W) {
		if (master_main_loop == 0) {
			regcmd.cmd &= ~DM_REG_W; //regcmd.cmd = send_data[0] & ~DM_REG_W;
			regcmd.data = send_data[1];
			regcmd.rdata = NULL_DONTCARE;
			if (regcmd.cmd >= DM9051_PAR && regcmd.cmd < DM9051_MAR) {
				blog_info("master send is [%02x][%02x]\r\n",regcmd.cmd, regcmd.data);
				//blog_info("master recv is [..][..]\r\n");
				master_main_loop = 1;
			}
		}
	}
	master_send_complete = 1;
}

void set_master_loop(void)
{
	master_main_loop = 0;
}

#define	TIMES_TO_RST	10
static void dm9051_show_rxbstatistic(uint8_t *htc, int n)
{
	int i, j;
	printf("SHW rxbStatistic, 254 wrngs\r\n");
	for (i = 0 ; i < (n+2); i++) {
		if (!(i%32) && i) printf("\r\n");
		if (!(i%32) || !(i%16)) printf("%02x:", i);
		//if (!(i%16)) printf(" ");
		if (!(i%8)) printf(" ");
		if (i==0 || i==1) {
			printf("  ");
			continue;
		}
		j = i - 2;
		printf("%d ", htc[j]);
	}
	printf("\r\n");
}

uint8_t ret_fire_time(uint8_t *histc, int csize, int i, uint8_t rxb)
{
	printf(" _dm9051f rxb %02x (times %2d)%c\r\n", rxb, histc[i], (histc[i]==2) ? '*': ' ');
	if (histc[i] >= TIMES_TO_RST) {
		dm9051_show_rxbstatistic(histc, csize);
		histc[i] = 1;
		return TIMES_TO_RST;
	}
	return 0;
}
