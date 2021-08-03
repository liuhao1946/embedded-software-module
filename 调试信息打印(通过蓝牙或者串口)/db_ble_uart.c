#include "db_ble_uart.h"

//这个数据结构是给你参考的，对你可能并没有用处
typedef struct
{
  uint16 handle;            
  uint8 len;                   
  uint8 value[247]; 
} attHandleValueNoti_t;
attHandleValueNoti_t db_notify_data;

//特别注意：
//在相关服务特征值通知被使能后，需要应用层手动让nofify_en_f = 1
//同理，在连接断开后,nofify_en_f = 0
uint8_t db_nofify_en_f;

batch_q_t db_bq;

void db_ble_notify_set(void )

void db_ble_clk(void )
{
#if DB_DATA_SEND_INTERFACE == DB_INTER_BLE
	uint16_t con_handle;
#endif
	
	int32_t len = 0;
	
 	DB_BLE_INT_PRO_DIS;
#if DB_DATA_SEND_INTERFACE == DB_INTER_BLE 
	if(db_nofify_en_f == DB_NOTI_ENABLE)
#endif
	{
		len = app_batch_data_dequeue(&db_bq, &db_notify_data.value[0], DB_DATA_PACK_MAX);
	}
	DB_BLE_INT_PRO_EN;
	
	if(len > 0)
	{
		db_notify_data.len = len;

		#if DB_DATA_SEND_INTERFACE == DB_INTER_BLE
        //注意：
        //这个地方是需要你更改为你的蓝牙数据发送接口
		    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &con_handle);
		    db_Notify(con_handle, &db_notify_data);
		#else
		    UART_SEND_BUFF(db_notify_data.value,db_notify_data.len);
		#endif
	}
	
	DB_BLE_INT_PRO_DIS;
	if(app_batch_queue_none(&db_bq) == 0)
	{
		APP_TIMER_STOP();
	}
	DB_BLE_INT_PRO_EN;
}

static void db_ble_send_str_inside(char *pstr, uint16_t len)
{
	uint8_t start_f = 0;
	int32_t data_len;
	
	if(app_batch_queue_none(&db_bq) == 0)
	{
		start_f = 1;
	}
	
	data_len = app_batch_data_enqueue(&db_bq,(uint8_t *)pstr, len);
	if(start_f && data_len > 0)
	{
		APP_TIMER_START();
	}
}

static void db_ble_print_inside(char *pformat, ...)
{
	char temp[64];
	va_list aptr;
	
    va_start(aptr, pformat);
    vsprintf(temp, pformat, aptr); 
    va_end(aptr);
	
	db_ble_send_str_inside(temp,strlen(temp));
}

void db_ble_print(char *pformat, ...)
{
#if BLE_NOTIFY_EN == 1
  	if(db_nofify_en_f != DB_NOTI_ENABLE)
	{
		return;
	}
#endif

	char temp[128];
	
	va_list aptr;
	
  	va_start(aptr, pformat);
  	vsprintf(temp, pformat, aptr); 
  	va_end(aptr);
	
	DB_BLE_INT_PRO_DIS;
	db_ble_send_str_inside(temp,strlen(temp));
  	DB_BLE_INT_PRO_EN;
}

void db_ble_send_str_ex(char *pstr,uint16_t len)
{
#if BLE_NOTIFY_EN == 1
	if(db_nofify_en_f != DB_NOTI_ENABLE)
	{
		return;
	}
#endif

	DB_BLE_INT_PRO_DIS;
	db_ble_send_str_inside(pstr, len);
  	DB_BLE_INT_PRO_EN;
}

void db_ble_send_str(char *pstr)
{
#if BLE_NOTIFY_EN == 1
	if(db_nofify_en_f != DB_NOTI_ENABLE)
	{
		return;
	}
#endif
	
	DB_BLE_INT_PRO_DIS;
	db_ble_send_str_inside(pstr,strlen(pstr));
	DB_BLE_INT_PRO_EN;
}

void db_ble_send_hex(char *str_head, uint8_t *dt, uint16_t len, char *str_end)
{
	uint16_t len_tmp;
	uint8_t start_f = 0;

#if BLE_NOTIFY_EN == 1
	if(db_nofify_en_f != DB_NOTI_ENABLE)
	{
		return;
	}
#endif

	if(app_batch_queue_none(&db_bq) == 0)
	{
		start_f = 1;
	}
	len_tmp = len;
	
	DB_BLE_INT_PRO_DIS;
	db_ble_send_str_inside(str_head, strlen(str_head));
	while(len_tmp--)
	{
		db_ble_print_inside("%02X ", *dt++);
	}
	db_ble_send_str_inside(str_end, strlen(str_end));
	DB_BLE_INT_PRO_EN;

	if(start_f && len > 0)
	{
		app_startup_db_ble_timer();
	}
}

void db_ble_send_dec_ex(char *str_head,int16_t *v, uint16_t len,char *str_end)
{
#if BLE_NOTIFY_EN == 1	
	if(db_nofify_en_f != DB_NOTI_ENABLE)
	{
		return;
	}
#endif

	DB_BLE_INT_PRO_DIS;
	db_ble_send_str_inside(str_head, strlen(str_head));
	while(len--)
	{
		db_ble_print_inside("%d,",*v++);
	}
	db_ble_send_str_inside(str_end, strlen(str_end));
	DB_BLE_INT_PRO_EN;
}

void db_ble_init(uint8_t *buf, uint16_t len)
{
	DB_BLE_INT_PRO_DIS;
	app_batch_queue_init(&db_bq, buf, 1, len);
	DB_BLE_INT_PRO_EN;
}



