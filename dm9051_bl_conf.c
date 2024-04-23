/**
  **************************************************************************
  * @file     dm9051_bl_conf.c
  * @version  v1.0
  * @date     2023-04-28, 2023-10-17, 2023-10-24, 2023-04-10
  * @brief    dm9051 config program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
#include <stdio.h>
#include "dm9051_bl.h"
#include "dm9051_bl_conf.h"

dm9051_interface_t dm9051_interface;

uint8_t send_data[CMD_LEN];
uint8_t recv_data[CMD_LEN];

//void dm9051_interface_register(dm9051_interface_t interface){
//    dm9051_if = interface;
//}

void dm9051_boards_initialize(hosal_spi_dev_t *pspi)
{
	spi_config(pspi);
    hosal_spi_irq_callback_set(pspi, spi_master_cb, (void*)pspi); /* register trans complete callback */
}

void spi_config(hosal_spi_dev_t *pspi)
{
	/* config spi */
    /* spi port set */
    pspi->port = 0;
	
    /* spi master mode */
    pspi->config.mode  = HOSAL_SPI_MODE_MASTER;  
   
    /* 1: enable dma, 0: disable dma */
    pspi->config.dma_enable = 0; //dma_enab;    
	
     /* 0: phase 0, polarity low                                           
      * 1: phase 1, polarity low                                           
      * 2: phase 0, polarity high                                          
      * 3: phase 0, polarity high
      */
    pspi->config.polar_phase= 0;     
	
    /* 0 ~ 40M */
    pspi->config.freq= DM9051_SPI_SPEED;
    pspi->config.pin_clk = DM9051_SPI_CLK;
    pspi->config.pin_mosi= DM9051_SPI_MOSI;
    pspi->config.pin_miso= DM9051_SPI_MISO;
	display_spi_info(pspi);
	
    /* init spi device */
    hosal_spi_init(pspi);
	
	/* register to driver */
	dm9051_interface.pspi = pspi;
}

void dm9051_boards_deinitialize(hosal_spi_dev_t *pspi)
{
	hosal_spi_finalize(pspi);
}

/*********************************
 * dm9051 spi interface accessing
 *********************************/
/* About Pin:
 * if pin % 4 is 0 ===> this pin can be used as spi mosi function
 * if pin % 4 is 1 ===> this pin can be used as spi miso function
 * if pin % 4 is 2 ===> this pin can be used as spi cs   function
 * if pin % 4 is 3 ===> this pin can be used as spi sclk function
 *
 * such as: GLB_GPIO_PIN_0 ===> mosi
 *          GLB_GPIO_PIN_1 ===> miso
 *          GLB_GPIO_PIN_2 ===> cs
 *          GLB_GPIO_PIN_3 ===> sclk
 *
 * about cs pin: for master device, user can use hardware cs pin like pin2,
 * and can also use software to select any pin for cs , 
 * for slave device ,user can only use hardwrae cs
 * about mosi and miso pin: mosi can be used as miso when miso can be uesd as mosi
 */

static void bl602_spi_cs(uint8_t value){
	hosal_spi_set_cs(DM9051_SPI_CS, value);
}
static uint8_t bl602_spi_write_and_read(hosal_spi_dev_t *pspi, uint8_t reg){
	send_data[0] = reg;
    hosal_spi_send_recv(pspi, (uint8_t *)send_data, (uint8_t *)recv_data, CMD_LEN, 10000);
	return recv_data[1]; /*send and recv data, timeout is 10s */
}
static void bl602_spi_write_and_write(hosal_spi_dev_t *pspi, uint8_t reg, uint8_t val){
	send_data[0] = reg; send_data[1] = val;
    hosal_spi_send_recv(pspi, (uint8_t *)send_data, (uint8_t *)recv_data, CMD_LEN, 10000);
} /*send and recv data, timeout is 10s */
static void bl602_spi_write(hosal_spi_dev_t *pspi, uint8_t dat){
	send_data[0] = dat;
	hosal_spi_send_recv(pspi, (uint8_t *)send_data, (uint8_t *)recv_data, 1, 10000);
}
static uint8_t bl602_spi_read(hosal_spi_dev_t *pspi){ //, uint8_t paddat //send_data[0] = paddat; //dummy
    hosal_spi_send_recv(pspi, (uint8_t *)send_data, (uint8_t *)recv_data, 1, 10000);
	return recv_data[0];
}

/*********************************
 * functions for driver's ops
 *********************************/

#define dm9051if_rstb_pulse()
#define dm9051if_cs_lo() bl602_spi_cs(0)
#define dm9051if_cs_hi() bl602_spi_cs(1)

//#define dm9051_spi_command_write(rd) spi_exc_data(rd)
//#define dm9051_spi_dummy_read() spi_exc_data(0)

//#define dm9051if_rstb_pulse() rst_pin_pulse()
//#define dm9051if_cs_lo() spi_cs_lo()
//#define dm9051if_cs_hi() spi_cs_hi()
//#define dm9051_spi_command_write(rd) spi_exc_data(rd)
//#define dm9051_spi_dummy_read() spi_exc_data(0)

#define dm9051_command_write_reg(cm, val)		bl602_spi_write_and_write(dm9051_interface.pspi, cm, val) //dm9051_if.spi_wrs(dm9051_if.pspi, cm, val)
#define dm9051_command_read_reg(cm)				bl602_spi_write_and_read(dm9051_interface.pspi, cm) //dm9051_if.spi_wr_and_rd(dm9051_if.pspi, cm)

#define dm9051_command_write(cm)				bl602_spi_write(dm9051_interface.pspi, cm) //dm9051_if.spi_wr(dm9051_if.pspi, cm)
#define dm9051_dummy_read()						bl602_spi_read(dm9051_interface.pspi) //dm9051_if.spi_rd(dm9051_if.pspi) //, 0

uint8_t cspi_read_reg(uint8_t reg) //static (todo)
{
#if 0
	uint8_t val;

	dm9051if_cs_lo();
	dm9051_spi_command_write(reg | OPC_REG_R);
	val = dm9051_spi_dummy_read();
	dm9051if_cs_hi();

	return val;
#endif
#if 1
    /* hardware cs now is pin 2 */
    /* software set pin4 as cs pin*/
	uint8_t spi_data;
    dm9051if_cs_lo(); //dm9051_if.cs_set(0);
	spi_data = dm9051_command_read_reg(reg | DM_REG_R); //dm9051_spi_command_write() + dm9051_spi_dummy_read()
    dm9051if_cs_hi(); //dm9051_if.cs_set(1);
	return spi_data;
#endif
}

void cspi_write_reg(uint8_t reg, uint8_t val)
{
    dm9051if_cs_lo(); //dm9051_if.cs_set(0);
	dm9051_command_write_reg(reg | DM_REG_W, val);
    dm9051if_cs_hi(); //dm9051_if.cs_set(1);
}

uint8_t cspi_read_mem2x(void)
{
	uint16_t i;
	union {
	uint8_t un;
	uint8_t rxb;
	} rd;
	dm9051if_cs_lo(); //dm9051_if.cs_set(0);
	dm9051_command_write(DM9051_MRCMDX | DM_REG_R); //dm9051_if.spi_wr(dm9051_if.pspi, DM9051_MRCMDX | DM_REG_R); //dm9051_spi_command_write(DM9051_MRCMDX | OPC_REG_R);
	for(i=0; i<2; i++)
		rd.rxb = dm9051_dummy_read(); //dm9051_if.spi_rd(dm9051_if.pspi, 0); //dm9051_spi_dummy_read();
	dm9051if_cs_hi(); //dm9051_if.cs_set(1);
	return rd.rxb;
}

void cspi_read_mem(uint8_t *buf, uint16_t len)
{
	uint16_t i;
	dm9051if_cs_lo(); //dm9051_if.cs_set(0);
	dm9051_command_write(DM9051_MRCMD | DM_REG_R); //dm9051_if.spi_wr(dm9051_if.pspi, DM9051_MRCMD | DM_REG_R); //dm9051_spi_command_write(DM9051_MRCMD | OPC_REG_R);
	if (len & 1)
		len++;
	for(i=0; i<len; i++)
		buf[i] = dm9051_dummy_read(); //dm9051_if.spi_rd(dm9051_if.pspi, 0); //dm9051_spi_dummy_read();
	dm9051if_cs_hi(); //dm9051_if.cs_set(1);
}

void cspi_write_mem(uint8_t *buf, uint16_t len)
{
	uint16_t i;
	dm9051if_cs_lo(); //dm9051_if.cs_set(0);
	dm9051_command_write(DM9051_MWCMD | DM_REG_W); //dm9051_if.spi_wr(dm9051_if.pspi, DM9051_MWCMD | DM_REG_W); //dm9051_spi_command_write(DM9051_MWCMD | OPC_REG_W);
	if (len & 1)
		len++;
	for(i=0; i < len; i++)
		dm9051_command_write(buf[i]); //dm9051_if.spi_wr(dm9051_if.pspi, buf[i]); //dm9051_spi_command_write(buf[i]);
	dm9051if_cs_hi(); //dm9051_if.cs_set(1);
}
