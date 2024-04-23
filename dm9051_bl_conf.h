/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __DM9051_ENV_H
#define __DM9051_ENV_H

#define DM9051_SPI_CS		4 //pin4 as cs pin
#define DM9051_SPI_CLK		3 //GPIO_PIN_3
#define DM9051_SPI_MISO		1 //PIN_1 ===> miso
#define DM9051_SPI_MOSI		0 //PIN_0 ===> mosi
#define DM9051_SPI_SPEED	100000

#define NULL_DONTCARE		0

void dm9051_boards_initialize(hosal_spi_dev_t *pspi);
void dm9051_boards_deinitialize(hosal_spi_dev_t *pspi);

#define bannerline_log()	printf("\r\n")
#define bannerdash_log(s)	printf("%s\r\n", s)

/*
 * dm9051 include files :
 */
typedef enum {
  DM_FALSE = 0,
  DM_TRUE = !DM_FALSE,
} enable_t;

void set_master_loop(void);

void spi_master_cb(void *arg);

void dm9051_show_lock_arch(const char *project_name);
void display_spi_info(hosal_spi_dev_t *pspi);
uint8_t ret_fire_time(uint8_t *histc, int csize, int i, uint8_t rxb);

void spi_config(hosal_spi_dev_t *pspi);

void dm9051_chipid(void);
void dm9051_init_setup(void);
void dm9051_start(const uint8_t *adr);
void hdlr_reset_process(const uint8_t *adr, enable_t en);

uint16_t ev_rxb(uint8_t rxb);

#define CMD_LEN				2

extern uint8_t send_data[CMD_LEN];
extern uint8_t recv_data[CMD_LEN];

#endif //__DM9051_ENV_H
