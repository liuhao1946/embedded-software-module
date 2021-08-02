#ifndef _APP_QUEUE_
#define _APP_QUEUE_

#include "stdint.h"

typedef enum
{
  	Q_NONE,
  	Q_NON_NONE,
  	Q_S_NONE,
  	Q_S_OK,
	Q_S_ERR,
}Q_status_t;

typedef uint16_t q_size_t;

typedef struct
{
	q_size_t head;
	q_size_t end;
	q_size_t q_len;
	q_size_t size;
}pos_t;

//初始化队列
//a_size: 队列中一个元素占用的字节数量
//q_max : 队列长度
//p_q   : 指向队列缓存
void app_queue_init(void *p_q, q_size_t q_max,q_size_t a_size);

//enqueue
void app_enqueue(void *p_q, void *p_src);

//dequeue
void app_dequeue(void *p_q, void *p_des);

//judge queue enpty
//if queue none ,return Q_NONE
uint8_t app_queue_none(void *p_q);

//search queue 
//if find, return Q_S_OK
uint8_t app_queue_search(void *p_q, void *p_src);

//clear queue all data
void app_queue_clr(void *p_q);

//获得队列元素个数
uint16_t app_queue_get_len(void *queue);

//获得队列剩余空间
uint16_t app_queue_get_remain_len(void *queue);

//遍历idx指定的队列元素
//注：idx = 0表示队列中第一个元素
//    idx = 1表示队列中第二个元素
//    idx = n表示队列中第n个元素，n不能超过队列中已有元素的长度
//如果找到idx对应的元素，元素内容会被拷贝到p_des指向的空间
//如果没有找到idx指定的元素,返回Q_S_ERR，否则Q_S_OK
Q_status_t app_queue_traverse(void *queue, void *p_des, q_size_t idx);

//将队列中位于索引idx的元素替换为p_replace
//如果成功替换，返回Q_S_ERR，否则Q_S_OK
Q_status_t app_queue_modify_speci_element(void *queue, q_size_t idx, void *p_replace);


//例子:
/*
#define NB_Q_MAX 10

typedef struct
{
	uint8_t tx_interval;
	uint8_t tx_try_cnt;
	uint16_t rx_timeout;
}nb_opt_t;

typedef struct
{
	pos_t  pos;  //必须有,且类型是pos_t
	nb_opt_t opt[NB_Q_MAX]; 
}nb_q_t;

nb_q_t nb_Q;

nb_opt_t test;

1、初始化队列
app_queue_init(&nb_Q, NB_Q_MAX, sizeof(nb_opt_t));

2、入队
app_enqueue(&nb_Q, &test);

3、出队
app_dequeue(&nb_Q, &p_des);

4、判断队列空
if(app_queue_none(&nb_Q) != Q_NONE)
{
}

5、搜索队列
if(app_queue_search(&nb_Q,&test) == Q_S_OK)
{
	队列中有相同内容
}

6、清除队列
app_queue_clr(&nb_Q);

7、找到队列中位于队列索引0位置的元素,结果保存到temp中
nb_opt_t temp;

app_queue_traverse(&nb_Q, &temp, 0);

8、找到对于中位于队列索引0位置的元素,将该元素替换为temp
nb_opt_t temp

temp.tx_interval = 0x01;
temp.tx_try_cnt = 0xff;

app_queue_modify_speci_element(&nb_Q, 0, &temp);

*/

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint16_t head;
	uint16_t end;
	uint16_t buf_size;
	uint16_t buf_pos;
	uint8_t size;
	void *data;
}batch_q_t;

/*
函数功能：获得队列的剩余长度
*/
uint16_t app_batch_queue_remain_len(batch_q_t *bq);

/*
函数功能：获得队列长度
*/
uint16_t app_batch_queue_get_data_len(batch_q_t *bq);

/*
函数功能：入队
*/
int32_t app_batch_data_enqueue(batch_q_t *bq, void *p, uint16_t len);

/*
函数功能：出队
*/
int32_t app_batch_data_dequeue(batch_q_t *bq, void *p, uint16_t len);

/*
函数功能：判断队列是否为空
	      函数返回0，表示队列为空
		  函数返回1，表示队列非空
*/
int16_t app_batch_queue_none(batch_q_t *bq);

/*
函数功能: 	批量数据队列初始化
			bq - 队列
			p - 指向应用层的队列缓存，缓存中每个数据大小只能是1字节、2字节、4字节
			size - 队列元素中每个数据大小(只能是1、2、4中的一个)，单位为字节
			len -  队列长度，要注意缓冲区长度必须是2的幂
*/
void app_batch_queue_init(batch_q_t *bq, void *p,uint8_t size, uint16_t len);


/*
例子：
batch_q_t bq_test;

uint8_t buf[1024]; //buf长度必须是2的倍数
uint8_t test[10] = {0x01,0x02,0x03,0x04,0x05};
uint8_t test_buf[200];

1、初始化
app_batch_queue_init(&bq_test,buf, 1,1024); //buf长度必须是2的倍数

如果队列缓存定义为：
uint32_t buf[1024];

app_batch_queue_init(&bq_test,buf, 4,1024);

2、入队
db_ble_data_enqueue(&bq_test, test, 5);

3、出队,有几种方式（假设队列中只有5个数据）

a、出队的数量低于5，则只出两个数据到test_buf
app_batch_data_dequeue(&bq_test,test_buf, 2);

b、出队数量≥5，只出5个数据

4、判断队列内还剩下多少空间
uin16_t len;

len = app_batch_queue_remain_len(&bq_test);

5、判断队列是否为空
//如果队列非空
if(app_batch_queue_none(&bq_test) != 0)
{
  .......
}

6、队列中有多少数据
app_batch_queue_get_data_len()

*/

#endif










