
#include "stm32f10x.h"
#include "misc.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

//RTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

TaskHandle_t task1_hd;
TaskHandle_t task2_hd;
TaskHandle_t task3_hd;
SemaphoreHandle_t key_sem;

void led_init(void )
{
  GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 
  GPIO_SetBits(GPIOB,GPIO_Pin_5);						

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		
  GPIO_Init(GPIOE, &GPIO_InitStructure);	  		
  GPIO_ResetBits(GPIOE,GPIO_Pin_5); 					
}

void uart_init(void )
{
	  //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		//USART2_TX   GPIOA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART2_RX	  GPIOA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;	

  USART_Init(USART1, &USART_InitStructure); 
  //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);           

	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	
	
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE); 
	
	//Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
}

void uart_send_buff_data(uint8_t *p, uint16_t len)
{
	while(len--)
	{
		USART_SendData(USART1, *p++);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
	}
}

void db_print(char *pformat, ...)
{
	char temp[256];
	va_list aptr;

  va_start(aptr, pformat);
  vsprintf(temp, pformat, aptr); 
  va_end(aptr);
	
	uart_send_buff_data((uint8_t *)temp, strlen(temp));
}

void USART2_IRQHandler(void)
{
	uint8_t rx_data;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;;
	
	rx_data = USART_ReceiveData(USART2);
	xTaskNotifyFromISR(task2_hd, rx_data, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void key_init(void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOE,ENABLE);
	//key0 - PE4
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //
	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI4_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	xSemaphoreGiveFromISR(key_sem,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	
	EXTI_ClearITPendingBit(EXTI_Line4);
}

void systick_init(void )
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(72000000 / 1000);
}

void vApplicationMallocFailedHook(void )
{
	db_print("malloc failed\n");
}

void vApplicationIdleHook(void )
{
	db_print("idel hook!\n");
}

void app_pre_sleep_process(TickType_t x)
{
	db_print("pre sleep\n");
}

void app_post_sleep_process(TickType_t x)
{
	db_print("post sleep\n");
}

void task1(void *p)
{
	static uint32_t key_timeout;

	for(;;)
	{
		if(key_timeout == 0)
		{
			db_print("key task block!\n");
			xSemaphoreTake(key_sem, portMAX_DELAY);
		}
		if(++key_timeout >= 10)
		{
			key_timeout = 0;
		}
		else
		{
			if(uxSemaphoreGetCount(key_sem) != 0)
			{
				xSemaphoreTake(key_sem, 0);
				key_timeout = 1;
			}
		}
		db_print("key task\n");
		
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

#define RX_BUF_LEN 256
uint8_t rx_buf[RX_BUF_LEN+1];

void task2(void *p)
{
	uint32_t rx_data;
//	static uint8_t rx_timeout;
	static uint16_t rx_pos;
	
	for(;;)
	{
		if(rx_pos == 0)
		{
			db_print("rx task block!\n");
			xTaskNotifyWait(0,0xff, &rx_data, portMAX_DELAY);
		}
		else
		{
			if(xTaskNotifyWait(0,0xff, &rx_data, pdMS_TO_TICKS(50)) != pdPASS)
			{
				rx_pos = 0;
				db_print("rx timeout:%s\n", rx_buf);
				continue;
			}
		}
		
		rx_buf[rx_pos++] = rx_data;
		rx_buf[rx_pos] = '\0';
		if(rx_pos >= RX_BUF_LEN)
		{
			rx_pos = RX_BUF_LEN-1;
		}
		
		//vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void task3(void *p)
{
	for(;;)
	{
		db_print("task3 run!\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	led_init();
	uart_init();
	key_init();

	if(xTaskCreate(task1, "task1", 200, 0, 5, &task1_hd) == pdPASS)
	{
		db_print("task1 creat suc!\n");
	}
	
	key_sem = xSemaphoreCreateCounting(3, 0);
	if(key_sem != NULL)
	{
		db_print("semaphore creat suc!\n");
	}
	
	xTaskCreate(task3, "task3", 200, 0, 7, &task3_hd);

	if(xTaskCreate(task2, "task2", 200, 0, 3, &task2_hd) == pdPASS)
	{
		db_print("task2 creat suc!\n");
	}
	db_print("run\n");
	
	vTaskStartScheduler();
	
	while(1)
	{	 
	}
}


