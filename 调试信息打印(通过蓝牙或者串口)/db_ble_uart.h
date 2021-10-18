#ifndef _BLE_DB_
#define _BLE_DB_

#include "stdint.h"

//////////////////////////////////////////////////////////////////////////////

//如果你用到串口发送数据，这个宏需要写上你的串口发送函数
#define UART_SEND_BUFF(a,b)  //hal_uart_send_buff(a,b)

/*
以下是应用层设置的定时器，定时器必须是循环的。定时间隔决定了db_ble_clk()的调用
间隔，这个函数负责把数据一包包的发出去，每包数据量最大值由DB_DATA_PACK_MAX决定

对于蓝牙，这个调用间隔不应该小于最小连接间隔(7.5ms)。DB_DATA_PACK_MAX默认是240
字节，因此，一个调用间隔内，不能有超过240字节的数据需要打印，否则入>出，可能造成
数据丢失

对于串口，则需要依据波特率设定定时间隔。比如9600波特率传输1字节需要1ms左右，那么
如果你每包数据100字节，那么定时间间隔至少要大于100ms
*/
#define APP_TIMER_START()     //app_startup_db_ble_timer()
#define APP_TIMER_STOP()      //app_stop_db_ble_timer();
#define APP_TIMER_GET_STATE() //获得定时器的开启状态

//////////////////////////////////////////////////////////////////////////////

//config
//每个时间间隔内一次发送多少字节出去
#define DB_DATA_PACK_MAX             240

//BLE_NOTIFY_EN = 1表示只发送BLE相关特征值通知使能后的数据
//BLE_NOTIFY_EN = 0表示即使BLE相关特征值通知没有使能，调试数据也会先进入
//缓冲区中保存，等到服务通知被使能后再把数据发送出去
#define BLE_NOTIFY_EN  0

//下面两个选项是选择发送数据的硬件接口，如果使用UART，需要让BLE_NOTIFY_EN = 0
#define DB_INTER_UART  1
#define DB_INTER_BLE   2

//中断保护使能.因为该模块使用了队列结构，如果在中断中以及在中断外都调用打印接口
//而不加保护，队列内的数据可能会乱掉。如果你只在中断外加入打印接口，可以不加保护
//要特别注意，如果使用RTOS，必须加入中断保护
#define DB_BLE_INT_PRO

#define DB_DATA_SEND_INTERFACE DB_INTER_BLE

#ifdef DB_BLE_INT_PRO
    #define DB_BLE_INT_PRO_DIS    __disable_irq() //总中断关闭
    #define DB_BLE_INT_PRO_EN     __enable_irq()  //总中断打开
#else
    #define DB_BLE_INT_PRO_EN    
    #define DB_BLE_INT_PRO_DIS 
#endif

extern uint8_t nofify_en_f;

void db_ble_init(uint8_t *buf, uint16_t len);

void db_ble_clk(void );

void db_ble_send_str(char *pstr);

void db_ble_send_str_ex(char *pstr,uint16_t len);

void db_ble_send_hex(char *str_head, uint8_t *dt, uint16_t len, char *str_end);

void db_ble_print(char *pformat, ...);

void db_ble_send_dec_ex(char *str_head,int16_t *v, uint16_t len, char *str_end);


/*
例子：

uint8_t buf[1024];  //buf大小必须是2的幂

1、初始化debug模块所需要的缓存
db_ble_init(buf,1024);

2、如果通过蓝牙接口发送数据，你需要做如下工作
2.1、DB_DATA_SEND_INTERFACE = DB_INTER_BLE
2.2、BLE_NOTIFY_EN = 0或者1(上面有说明)
2.3、在蓝牙连接后：nofify_en_f = 1
2.4、在蓝牙断开后：nofify_en_f = 0
2.5、更改db_ble_uart.c中db_ble_clk()函数中的蓝牙数据发送接口(45~46行)

3、如果通过串口发送，你需要做如下工作
3.1、DB_DATA_SEND_INTERFACE = DB_INTER_UART
3.2、BLE_NOTIFY_EN = 0
3.3、UART_SEND_BUFF(a,b)替换为你的串口发送接口 

4、在外部定时器中调用db_ble_clk()，并且把APP_TIMER_START()、APP_TIMER_STOP()
替换为该外部定时器打开与关闭函数

例如：
外部定时器(一般是软件定时器)为：
void timer_interrupt(void )
{
    db_ble_clk();
}
//外部定时器启动
void timer_startup(void )
{
}
//外部定时器关闭
void timer_stop(void )
{
}

#define APP_TIMER_START() timer_startup()
#define APP_TIMER_STOP()  timer_stop();

接下来可以使用信息打印接口了
1、打印字符串，3个接口
1.1、db_ble_print("test\r\n");  
1.2、db_ble_send_str("test\r\n");
1.3、db_ble_send_str_ex("test\r\n",6);

注意：
db_ble_print()函数用来打印总长超过128字节的字符串会造成严重问题，如果要
打印超过128字节字符串，可以使用1.2、1.3，这两个接口没有长度限制(除非本身溢出)

2、打印浮点数、整形数、十六进制等（类似C标准的printf）
float t1 = 0.2334;
int32_t t2 = -10;

db_ble_print("t2=%d,t1=%0.2f\n", t2,t1);
结果：
t2=-10,t1=0.23

3、批量打印十六进制数据
uint8_t test[10] = {0x01,0x02,0x03,0x04,0xa5};

db_ble_send_hex("rx:",test,5,"\r\n");
结果：
rx:01 02 03 04 a5

db_ble_send_hex("",test,5," kk\r\n");
结果：
01 02 03 04 A5 kk

4、批量打印16位有符号数据
int16_t test[10] = {-22,22,120};

db_ble_send_dec_ex("rx:",test,3," pp\r\n");
结果：
rx:-22,22,120 pp

db_ble_send_dec_ex("rx:",test,3,"");
结果：
rx:-22,22,120
*/

#endif

