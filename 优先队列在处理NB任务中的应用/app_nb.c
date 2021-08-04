#include "app_nb.h"
#include "string.h"

#define NB_TASK_MAX_NUM 16


uint8_t nb_evt_step = 0;
uint8_t nb_task_st = NB_TASK_IDEL;

///////////////////////////////////////////////////////
//nb task
void app_nb_hal_startup(void );
void app_nb_tx_cmd_at(void );

//error
uint8_t app_nb_tx_cmd_at_err(void );

///////////////////////////////////////////////////////
const nb_err_t nb_err[] = 
{
	{0},
	{app_nb_tx_cmd_at_err},
};

//////////////////////////////////////////////////////////////

typedef struct _NB_LIST_
{
	nb_opt_t para;
	struct _NB_LIST_ *next;
}nb_task_t;

static nb_task_t nb_task[NB_TASK_MAX_NUM];
static nb_task_t *free_list;
static nb_task_t nb_list_head;
static nb_task_t *nb_task_head = &nb_list_head;
static nb_opt_t *task;

/*
把nb_task[]数组初始化为自由链表。

在这里，链表分为两部分，一部分是自由链表，一部分是工作链表。

所谓自由链表就是对应用层来说没有实际用处的链表。他的作用是当我们需要构成一个
链表以完成一些功能时，就从这个链表中取出一个位置，由这些有功能的链表构成的另外
一个链表就是工作链表，NB的所有任务都处于工作链表中。
*/
void app_nb_task_list_init(void)
{
	uint16_t i;
	
	nb_task_t *plst;
	nb_task_t *qlst;

	plst = &nb_task[0];
	qlst = &nb_task[1];
	for (i = 0; i < NB_TASK_MAX_NUM - 1; i++)
	{
		plst->para.prior = 0;
		plst->next = qlst;
		++plst;
		++qlst;
	}
	plst->next = 0;
	free_list = &nb_task[0];
}

/*
从自由链表中取出一个位置用来存放用户数据。注意，这里的存放方式是：

1、按优先级存储(pdt->prior)，这个变量值越大，nb任务的优先级越低。pdt->prior=0优先级最高

2、当优先级相同时，按照入链顺序依次排列，此时是一个先进先出的队列结构
*/
void app_nb_task_list_reg(nb_task_t **phead, nb_opt_t *pdt)
{
	nb_task_t *plst;
	nb_task_t *qlst;
	uint8_t task_prior;

	if (free_list == 0)
	{
		return;
	}
	task_prior = pdt->prior;
	plst = *phead;
	qlst = free_list;
	free_list = qlst->next;
	while (plst->next != 0 && plst->next->para.prior <= task_prior)
	{
		plst = plst->next;
	}

	qlst->next = plst->next;
	plst->next = qlst;
	memcpy(&qlst->para, pdt, sizeof(nb_opt_t)/sizeof(uint8_t));
}

/*
删除工作链表。删除后，这些工作链表被回收到自由链表中
*/
void app_nb_list_del_all(nb_task_t **phead)
{
	nb_task_t *plst;
	nb_task_t *qlst;

	plst = (*phead)->next;
	if (plst == 0)
	{
		return;
	}
	while (plst->next != 0)
	{
		plst = plst->next;
	}
	qlst = free_list;
	plst->next = qlst;
	free_list = (*phead)->next;
	(*phead)->next = 0;
}

/*
删除链表中的指定任务。被删除后的任务所用的链表被回收到自由链表中
*/
void app_nb_list_del_speci(nb_task_t **phead, nb_opt_t *ptask)
{
	nb_task_t *plst;
	nb_task_t *qlst;

	if((*phead)->next == 0 || ptask == 0)
	{
		return;
	}
	
	plst = (*phead);
	while (plst->next != 0 && plst->next->para.tx != ptask->tx)
	{
		plst = plst->next;
	}
	if (plst->next != 0)
	{
		qlst = plst->next;
		plst->next = plst->next->next;
		qlst->next = free_list;
		free_list = qlst;
		
		memset(&qlst->para, 0, sizeof(nb_opt_t) / sizeof(uint8_t));
	}
}

/*
搜索工作链表中是否的指定任务，如果找到会把指定任务的相关数据复制到ptask。如果检索到目标，
函数返回0，其情况返回1
*/
uint8_t app_nb_search_speci_task(nb_task_t *phead, nb_tx_cb_t tx_cb, nb_opt_t *ptask)
{
	nb_task_t *plst;

	plst = phead->next;
	if (plst == 0)
	{
		return 1;
	}

	while (plst != 0 && plst->para.tx != tx_cb)
	{
		plst = plst->next;
	}
	if (plst != 0)
	{
		memcpy(ptask, &plst->para, sizeof(nb_opt_t) / sizeof(uint8_t));
		return 0;
	}

	return 1;
}

/*
工作链是否为空。如果工作链为空，返回LIST_NULL，否则返回LIST_NON_NULL
*/
nb_list_st_t app_nb_task_none(nb_task_t *phead)
{
	if (phead->next == 0)
	{
		return LIST_NULL;
	}

	return LIST_NON_NULL;
}

/***************************************************
* 启动NB模块任务
****************************************************/
void app_nb_hal_startup_done(char *pstr,uint16_t tm)
{
    NB_POWERKEY_H;
}

void app_nb_hal_startup(void )
{
	nb_opt_t opt;
	
	opt.tx = app_nb_hal_startup_done;
	opt.tx_interval = app_time_set_clk_ms(800);
	opt.tx_try_cnt = 1;
	opt.pstr = 0;
	opt.prior = 2;
	
	opt.rx = 0;
	opt.rx_timeout = 0;
	opt.err = NB_ERR_NONE;
	
	app_nb_list_del_speci(&nb_task_head, &opt);
	app_nb_task_list_reg(&nb_task_head, &opt);

	NB_POWERKEY_L;
}

/******************************************************************
发送AT指令
*******************************************************************/
uint8_t app_nb_cmd_at_ack(nb_data_t *nb_dt)
{
	if(strstr((char *)nb_dt->dt, "\r\nOK") != 0)
	{
		return NB_SUC;
	}
	else
	{
		return NB_ERR;
	}
}

void app_nb_cmd_at(char *pstr, uint16_t tm )
{
	unsigned char cmd[] = "AT\r\n";
	
	SEND_NB_CMD(cmd, sizeof(cmd), tm);
}

void app_nb_tx_cmd_at(void )
{
	nb_opt_t opt;
	
	opt.tx = app_nb_cmd_at;
	opt.tx_interval = app_time_set_clk_ms(100);
	opt.tx_try_cnt = 10;
	opt.pstr = 0;
	
	opt.rx = app_nb_cmd_at_ack;
	opt.rx_timeout = app_time_set_clk_ms(500);
	
	opt.err = NB_ERR_AT_CMD_TIMEOUT;
	
	opt.prior = 2;
	
	app_nb_list_del_speci(&nb_task_head, &opt);
	app_nb_task_list_reg(&nb_task_head, &opt);
}

uint8_t app_nb_tx_cmd_at_err(void )
{
	app_nb_list_del_all(&nb_task_head);
    
    /*如果指令出现错误，你可以在这里重启模块、初始化模块参数等*/
	
	return 0;
}

void app_nb_init(void )
{
	app_nb_hal_startup();
	app_nb_tx_cmd_at();
}

void app_nb_task_clr(void )
{
	app_nb_list_del_all(&nb_task_head);
	nb_evt_step = 0;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

/*
这个函数需要在串口中调用。当MCU接收模块发过来的完整数据后，就调用该接口，以便MCU完成相关
的指令匹配
*/
void app_nb_rx_handle(uint8_t *p, uint16_t len)
{
	if(task != 0 && task->rx != 0)
	{
		nb_data_t nb;
		
		nb.dt = p;
		nb.len = len;
		
		if(task->rx(&nb) == NB_SUC)
		{
			nb_evt_step = 1;
		    app_nb_list_del_speci(&nb_task_head, task);
		}
	}
}

/*
这个函数完成以下几个工作：
1、调度NB任务，如果出现优先级高的任务，在上一条指令执行完成后(接收超时或者NB模块对相应指令做了应答)，
立刻执行当前优先级最高的任务

2、当任务执行n（参考app_nb_tx_cmd_at()函数中的opt.tx_try_cnt决定）次后，都没有收到模块应答或者
收到了模块的错误应答，将根据nb_err[]中的设置进行错误处理(比如初始化参数、重启模块等)。如果第一次
发送指令模块给出了正确应答，则MCU继续下一个任务，直到不再有其他任务

3、这个函数的调用间隔决定了指令发送间隔与应答超时时间。比如该函数的调用间隔是10ms，发送AT指令后，该
指令的应答时间应该小于300ms，此时opt.rx_timeout=30(30*10ms=300ms)
*/
void app_nb_clk(void )
{
	static uint32_t rx_timeout = 0;
	
	switch(nb_evt_step)
	{
		case 0:
			if(app_nb_task_none(nb_task_head) != LIST_NULL)
	        {
				nb_evt_step = 1;
				nb_task_st = NB_TASK_BUSY;
				//wakeup 
				NB_PSM_WAKEUP_L;
	        }
			else
			{
				NB_PSM_WAKEUP_H;
				nb_task_st = NB_TASK_IDEL;
			}
			break;
		
		case 1:
		{
			if(app_nb_task_none(nb_task_head) == LIST_NULL)
			{
				nb_evt_step = 0;
				break;
			}
			
		    task = &nb_task_head->next->para;
			if(task->tx_interval)
			{
				task->tx_interval--;
				break;
			}
			
			if(task->tx_try_cnt != 0)
			{
				if(task->tx != 0)
				{
					task->tx(task->pstr,task->rx_timeout);
				}
				
				if(task->rx != 0)
				{
					rx_timeout = task->rx_timeout;
					nb_evt_step = 2;
				}
				else
				{
					app_nb_list_del_speci(&nb_task_head, task);
				}
			}
			else
			{
				 if(task->err < NB_ERR_END && nb_err[task->err].cb != 0)
				 {
					nb_err[task->err].cb();
				 }//timeout
		
				 app_nb_list_del_speci(&nb_task_head, task);
				 task = 0;
			}
			
			if(task != 0 && task->tx_try_cnt)
			{
				task->tx_try_cnt--;
			}
	    }
			break;
		
		case 2:
			if(rx_timeout != 0)
	        {
		        rx_timeout--;
	        }
			else
			{ 
				nb_evt_step = 1;
			}//timeout
			break;
	}

}

///////////////////////////////////////////////////////////////
//模块
main()
{
    app_nb_init();
    while(1)
    {
        if(clk_50ms == 1)
        {
            clk_50ms = 0;
            app_nb_clk();
        }
    }
}