/**
 * Copyright (c) 2023-2025 Davicom Semiconduct., Inc.
 * 20230830 Joseph CHANG
 */
#include <stdio.h>
#include "dm9051_bl.h"
#include "dm9051_bl_conf.h"

/**
  * @brief  updates the link states
  * @retval link state 0: disconnect, 1: connection
  */
uint16_t dm9051_bmsr_update(void)
{
	uint16_t link_state;
//	do {
//		uint16_t link_data;
//		link_data = phy_read(PHY_STATUS_REG);
//		link_state = (link_data & PHY_LINKED_BIT) >> 2;
//	} while(0);
	do {
		uint8_t nsr = DM9051_Read_Reg(DM9051_NSR);
		link_state = (nsr  & NSR_LINKST) ? 1 : 0;
		dm9051_update_flags(nsr);
	} while(0);
	return link_state;
}

//void dm9051_init(const uint8_t *adr)
const uint8_t *dm9051_init(const uint8_t *adr)
{
	dm9051_show_lock_arch("bl602_web-demo");
	
	dm9051_init_setup();
	hdlr_reset_process(adr, DM_TRUE);
	return adr;
}

#define DM9051_RX_BREAK(expression, handler) do { if ((expression)) { \
  handler;}} while(0)

static uint16_t buff_rx(uint8_t *buff)
{
	uint16_t rx_len = 0;
	uint8_t rxbyte, rx_status;
	uint8_t ReceiveData[4];
	
	rxbyte = DM9051_Read_Mem2X(); //DM9051_Read_Rxb(); //DM9051_Read_Reg(DM9051_MRCMDX);
	//DM9051_RXB_Basic(rxbyte); //(todo) Need sevice case.
	
	DM9051_RX_BREAK((rxbyte != 0x01 && rxbyte != 0), return ev_rxb(rxbyte));
	DM9051_RX_BREAK((rxbyte == 0), return 0);
		
	DM9051_Read_Mem(ReceiveData, 4);
	DM9051_Write_Reg(DM9051_ISR, 0x80);
	
	rx_status = ReceiveData[1];
	rx_len = ReceiveData[2] + (ReceiveData[3] << 8);
	
	DM9051_RX_BREAK((rx_status & 0xbf), return err_hdlr("_dm9051f rx_status error : 0x%02x\r\n", rx_status, 0));
	//instead of : RX_POOL_BUFSIZE = (1514+4)
	DM9051_RX_BREAK((rx_len > PBUF_POOL_BUFSIZE), return err_hdlr("_dm9051f rx_len error : %u\r\n", rx_len, 0));

	DM9051_Read_Mem(buff, rx_len);
	DM9051_Write_Reg(DM9051_ISR, 0x80);
	return rx_len;
}

uint16_t dm9051_rx(uint8_t *buff)
{
	if (!dm9051_get_flags()) {
		dm9051_unlink_inc();
		#if 1
//		do {
//			uint8_t nsr = DM9051_Read_Reg(DM9051_NSR);
//			dm9051_update_flags(nsr);
//		} while(0);
		dm9051_bmsr_update();
		#endif
		return 0;
	}
	return buff_rx(buff);
}

#define DM9051_TX_DELAY(expression, handler) do { if ((expression)) { \
  handler;}} while(0)

void dm9051_tx(uint8_t *buf, uint16_t len)
{
	DM9051_Write_Reg(DM9051_TXPLL, len & 0xff);
	DM9051_Write_Reg(DM9051_TXPLH, (len >> 8) & 0xff);
	DM9051_Write_Mem(buf, len);
	DM9051_Write_Reg(DM9051_TCR, TCR_TXREQ); /* Cleared after TX complete */
	DM9051_TX_DELAY((DM9051_Read_Reg(DM9051_TCR) & TCR_TXREQ), vTaskDelay(5) /*dm9051_if.delay_ticks(5)*/); //dm_delay_us(5)
}
