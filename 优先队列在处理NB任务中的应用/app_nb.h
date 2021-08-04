
#ifndef _APP_NB_
#define _APP_NB_

#include "app_common.h"
#include "app_queue.h"
#include "app_uart.h"
#include "gpio.h"
#include "uart.h"
#include "app_datetime.h"


/////////////////////////////////////////////////////////////////
//p: need send data
//len: data length
//tm: nb cmd timeout time, use for mcu uart at the right time to sleep
extern void app_uart_tx(uint8_t *p, uint16_t len, uint16_t tm);
extern void app_gpio_nb_powerkey(uint8_t state);
extern void app_gpio_nb_psm_wakeup(uint8_t state);
extern void app_gpio_gps_ant_power_ctr(uint8_t state);

//extern void app_inte_rx_nb_data_handle(uint8_t *p, uint16_t len);

#define NB_COM_EN  2  

#define NB_POWERKEY   GPIO_P09
#define NB_PSM_WAKEUP GPIO_P10
#define NB_POWER_EN   GPIO_P06

#define NB_POWERKEY_H 	do{app_gpio_nb_powerkey(0);}while(0)
#define NB_POWERKEY_L 	do{app_gpio_nb_powerkey(1);}while(0)

#define NB_PSM_WAKEUP_H   do{app_gpio_nb_psm_wakeup(0);}while(0)
#define NB_PSM_WAKEUP_L   do{app_gpio_nb_psm_wakeup(1);}while(0)

#define NB_GPS_ANT_POWER_ON() app_gpio_gps_ant_power_ctr(1)
#define NB_GPS_ANT_POWER_OFF() app_gpio_gps_ant_power_ctr(0)

#define SEND_NB_CMD(a,b,c) app_uart_tx((uint8_t *)a,b,c)

//#define APP_NB_REC_HANDLE(a,b) app_inte_rx_nb_data_handle(a,b)

//////////////////////////////////////////////////////////////////

#define NB_Q_MAX    16
#define NB_RX_BUF   512
#define NB_TX_BUF   1280

//#define NB_GPS_EN

typedef enum
{
	NB_SUC,
	NB_ERR,
}nb_opt_status;

typedef enum
{
  LIST_NULL,
	LIST_NON_NULL
}nb_list_st_t;

typedef enum
{
	NB_SEND_STATUS_SUC,
	NB_SEND_STATUS_FAIL,
	NB_SEND_STATUS_WAIT
}nb_send_st_t;

typedef enum
{
	NB_INIT_START,
	NB_INIT_WAIT,
	NB_INIT_DONE
}nb_init_status_t;

typedef enum
{
	NB_READY,
	NB_WAIT,
	NB_UNKNOWN,
}nb_status_t;

typedef enum
{
	PSM_EN,
	PSM_DIS
}psm_cf_t;

typedef enum
{
	EDRX_EN,
	EDRX_DIS
}edrx_cf_t;

typedef enum
{
	GPS_LOC_INVALID = (uint8_t)0,
	GPS_LOC_VALID,
	GPS_LOC_UNKNOWN,
}gps_status_t;

typedef enum
{
	GPS_PW_EN,
	GPS_PW_DIS,
}nb_gps_pw_st_t;

typedef enum
{
	GPS_DISBEL,
	GPS_EN,
}gps_ctr_t;

typedef  enum
{
	NB_ERR_NONE,
	NB_ERR_AT_CMD_TIMEOUT,
	NB_ERR_AT_QLWDATASEND_CMD_TIMEOUT,
	NB_ERR_AT_GET_GPS_LOCATION,
	NB_ERR_SUB_INFO,
	
	NB_ERR_END
}nb_err_num_t;

typedef enum
{
	CEREG_OPT_SET,
	CEREG_QUERY,
}cereg_opt_t;

typedef enum
{
	NB_TASK_IDEL,
	NB_TASK_BUSY,
}nb_task_status_t;

typedef enum
{
	NB_ECHO_EN,
	NB_ECHO_DIS
}nb_echo_set_t;

typedef struct
{
	uint8_t data[5];
	uint8_t len;
}str_d_t;

typedef struct
{
	uint8_t *dt;
	uint16_t len;
}nb_data_t;

typedef enum
{
	TIME_SYN_START,
	TIME_SYN_DONE,
}time_syn_t;

typedef enum
{
	NB_POWER_UP,
	NB_POWER_DOWN_START,
	NB_POWER_DOWN_DONE,
}nb_power_st_t;


typedef void (*nb_read_cb_t)(uint8_t *p, uint16_t len);

typedef void (*p_notify_send_status_cb_t)(uint8_t state);

typedef struct 
{
	p_notify_send_status_cb_t cb;
	uint8_t *p;
	uint16_t len;
}nb_tx_data_t;

typedef struct
{
	uint8_t latitude[10];
	uint8_t longitude[11];
	uint8_t lat_dir;
	uint8_t long_dir;
	uint8_t satellite_sum;
	gps_status_t location_status;
}gps_coordinate_t;

typedef uint8_t (*nb_rx_cb_t)(nb_data_t *p);
typedef void (*nb_tx_cb_t)(char *pstr,uint16_t tm);

typedef struct
{
//	uint8_t *cmd;  
	nb_tx_cb_t tx;
	nb_rx_cb_t rx;
	char *pstr;
  uint16_t rx_timeout;
	uint16_t tx_interval;
	uint8_t tx_try_cnt;
	uint8_t prior;
	uint8_t err;
	
}nb_opt_t;

typedef uint8_t (* nb_err_cb_t)(void );

typedef struct
{
  nb_err_cb_t cb;
}nb_err_t;

typedef struct
{
	uint8_t sim[30];
	uint8_t imei[30];
}nb_id_t;

typedef struct
{
	pos_t  pos;
	nb_opt_t opt[NB_Q_MAX]; 
}nb_q_t;


///////////////////////////////////////////////////////////////
extern void app_nb_rx_handle(uint8_t *p, uint16_t len);
extern void app_nb_clk(void );
extern void app_nb_init(void );
///////////////////////////////////////////////////////////////

extern void app_nb_tx_cmd_at_qlwdatasend(nb_tx_data_t *ptx, uint8_t cnt);

extern uint8_t app_nb_gps_get_satellite_amount(void );

extern void app_nb_gps_startup(void );

extern gps_status_t app_nb_gps_get_status(void );

extern uint8_t app_nb_gps_get_satellite_amount(void );

void app_nb_get_sim_ccid(nb_id_t *p);

void app_nb_get_emei(nb_id_t *p);

//get gps location
void app_nb_gps_get_location(gps_coordinate_t *p);

//startup gps read 
void app_nb_gps_read_startup(void );

//get nb data send status
nb_send_st_t app_nb_get_send_status(void );

//get gps power status 
nb_gps_pw_st_t app_nb_gps_get_power_status(void );

//get NB task status
nb_task_status_t app_nb_get_task_status(void );

//get NB module status
nb_status_t app_nb_get_status(void );

//get gps module status
gps_status_t app_nb_gps_get_status(void );

//send nb cclk cmd, get nb time
void app_nb_tx_cmd_at_cclk(p_notify_send_status_cb_t cb);

//get nb time
void app_nb_get_nb_time(datetime_t *pclock);

void app_nb_tx_cmd_qpowd(p_notify_send_status_cb_t  cb);

void app_nb_tx_cmd_at_qlwrd(nb_read_cb_t cb,uint8_t read_cnt);

void app_nb_gps_clr_status(void );

void app_nb_task_clr(void );

void app_nb_task_list_init(void);

nb_power_st_t app_nb_get_power_status(void );

#endif


