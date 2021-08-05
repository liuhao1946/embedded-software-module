
#ifndef _APP_NB_
#define _APP_NB_

#include "stdint.h"


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


/*
说明：
1、这个模块的使用方法见app_nb.c中433行

2、模块中代码的解释见app_nb.c相应函数的描述

3、这个模块可以实现同优先级任务按照先入先出的队列出队。当有更高优先级任务出现，更高优先级
任务会抢占低优先级任务，等到高优先级任务执行完成，在接着低优先级任务执行

比如，我要发送一个AT命令，重复发送的次数设置为10次并假设模块一直没有应答。在AT命令执行到第3次时，
有一个更高优先级的任务出现，此时并不会立刻执行这个优先级最高的任务，而是需要等到第三次的AT指令超
时后才能执行该优先级最高的任务。当这个高优先级的任务执行完成，又开始执行回AT指令，此时AT指令是接
着第3次发送，也就是第4次重发指令

*/


#endif


