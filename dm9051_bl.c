/**
 * Copyright (c) 2023-2025 Davicom Semiconduct., Inc.
 * 20230830 Joseph CHANG
 */
#include <stdio.h>
#include <lwip/err.h>
#include "include/develop.h"
#include "dm9051_bl.h"
#include "dm9051_bl_conf.h"

#define UNLINK_COUNT_RESET	60000

static uint16_t unlink_count;

void dm9051_unlink_inc(void) {
	if (unlink_count < UNLINK_COUNT_RESET)
		unlink_count++;
}

//static void test_DM9051_Write_Reg(uint8_t reg, const uint8_t *pdata, int bufsize) {
//    dm9051_if.cs_set(0);
//	for (int i= 0; i< bufsize; i++)
//		dm9051_command_write_reg((reg+i) | DM_REG_W, *pdata++);
//    dm9051_if.cs_set(1);
//}

static void dm9051_set_par(const uint8_t *macadd){
#if 1
	int i;
	for (i=0; i<6; i++)
		DM9051_Write_Reg(DM9051_PAR+i, macadd[i]);
#else
//	test_DM9051_Write_Reg(DM9051_PAR, macadd, 6);
#endif
}

static void dm9051_set_mar(void){
	int i;
	for (i=0; i<8; i++)
		DM9051_Write_Reg(DM9051_MAR+i, (i == 7) ? 0x80 : 0x00);
}

static void dm9051_set_recv(void){
#if 0
	DM9051_Write_Reg(DM9051_FCR, FCR_DEFAULT); //iow(DM9051_FCR, FCR_FLOW_ENABLE);
	phy_write 04, flow
#endif
	DM9051_Write_Reg(DM9051_IMR, IMR_PAR | IMR_PRM); //iow(DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
	DM9051_Write_Reg(DM9051_RCR, RCR_DEFAULT | RCR_RXEN); //dm9051_fifo_RX_enable();
}

static void dm9051_core_reset(void)
{
	DM9051_Write_Reg(DM9051_GPR, 0x00);		//Power on PHY
	vTaskDelay(1*50); //dm9051_if.delay_ticks(1*50); //dm_delay_ms(1);
	
	dm9051_update_flags(~NSR_LINKST); //= dm9051_clear_flags(lw_flag, DM9051_FLAG_LINK_UP);
	unlink_count = 0;

	DM9051_Write_Reg(DM9051_NCR, DM9051_REG_RESET); //iow(DM9051_NCR, NCR_RST);
	//printf("DM9051_MBNDRY: MBNDRY_WORD\r\n");
	DM9051_Write_Reg(DM9051_MBNDRY, MBNDRY_WORD); /* MemBound */
	DM9051_Write_Reg(DM9051_PPCR, PPCR_PAUSE_COUNT); //iow(DM9051_PPCR, PPCR_SETTING); //#define PPCR_SETTING 0x08
	DM9051_Write_Reg(DM9051_LMCR, LMCR_MODE1);
}

void dm9051_mac_adr(const uint8_t *macadd) {
	dm9051_set_par(macadd); //show_par();
}

void dm9051_rx_mode(void) {	
	dm9051_set_mar(); //show_mar();
	dm9051_set_recv();
}

void dm9051_start(const uint8_t *adr)
{
	dm9051_mac_adr(adr);
	dm9051_rx_mode();
}

char *display_mac_bannerline_defaultN = ": Display Bare device";

//static 
void dm9051_chipid(void)
{
	uint8_t *ids = NULL;
	uint16_t chipid;
	uint8_t rev;

	chipid = (uint32_t)DM9051_Read_Reg(DM9051_PIDH) << 8 | (uint32_t)DM9051_Read_Reg(DM9051_PIDL);
	if (chipid != 0x9051) {
		printf("DM9051 not found, chipid: %04x\r\n", chipid);
		printf("system stop\r\n");
		while(1);
	}
	
	bannerdash_log("       ...");
	bannerline_log();
	bannerdash_log("       ..."); //printf("       ... \r\n");
	bannerdash_log("       ..."); //printf("       ... \r\n");
	bannerdash_log("       ..."); //printf("       ... \r\n");
	bannerdash_log("       ..."); //printf("       ... \r\n");
	bannerdash_log("       ..."); //printf("       ... \r\n");
	bannerline_log();
	bannerdash_log("       ...");
	bannerline_log();
	
	printf("DM9051 chipid found:  	%04x\r\n", chipid);
	rev = DM9051_Read_Reg(0x5c);
	printf("DM9051 chip rev:      	%02x\r\n", rev);
	
	bannerline_log();
	
	printf("%s[%d] ::: (%s) chip rev %02x, Chip ID %04x (CS_EACH_MODE)%s\r\n",
		display_mac_bannerline_defaultN, //display_identity_bannerline_title ? display_identity_bannerline_title : display_identity_bannerline_default,
		0, //mstep_get_net_index(),
//		psh_ids1[mstep_get_net_index()][0], psh_ids1[mstep_get_net_index()][1],
//		psh_ids1[mstep_get_net_index()][2], psh_ids1[mstep_get_net_index()][3], 
		"(CS_EACH_MODE)", //DM_GET_DESC(csmode_t, csmode), //dm9051opts_desccsmode()
		rev, chipid,//psh_id_adv1[mstep_get_net_index()], psh_id1[mstep_get_net_index()],
		ids ? "" : ".(Rst.process)");
}

void dm9051_init_setup(void)
{
	dm9051_chipid(); //param(hosal_spi_dev_t *pspi,) //dm9051_chipid(pspi);
}

void hdlr_reset_process(const uint8_t *adr, enable_t en)
{
	dm9051_core_reset();
	dm9051_start(adr);
}

// -------------------------------------------------- process ---------------------------------------------

void dm9051_core_process(void)
{
	dm9051_core_reset(); //As: printf("rstc %d ,because rxb %02x (is %d times)\r\n", rstc, rxbyte, times);
	dm9051_delay_in_core_process(300);
	dm9051_set_recv(); //of _dm9051_rx_mode();
}

//static
uint16_t err_hdlr(char *errstr, uint32_t valuecode, uint8_t zerochk)
{
	if (zerochk && valuecode == 0)
		return 0;
	printf(errstr, valuecode);
	dm9051_core_process();
	return 0;
}

//static
uint16_t ev_rxb(uint8_t rxb)
{
	int i;
	static uint8_t histc[254] = { 0 }; //static int rff_c = 0 ...;
	uint8_t times = 1;
	
	for (i = 0 ; i < sizeof(histc); i++) {
		if (rxb == (i+2)) {
			histc[i]++;
			times = ret_fire_time(histc, sizeof(histc), i, rxb);
			err_hdlr("_dm9051f rxb error times : %u\r\n", times, 1); //As: Hdlr (times : TIMES_TO_RST or 0)
		}
	}
	return err_hdlr("_dm9051f rxb error times : %u\r\n", times, 0); //As: Hdlr (times : 1)
}
