#ifndef _APP_DATETIME_
#define _APP_DATETIME_


#include "stdint.h"

//RTC使用的低频晶振频率，一般是32000或者32768
#define RTC_FREQUEN       32768
//RTC寄存器的最大值
#define RTC_CNT_REG_MAX   0x00ffffff
/*
获得RTC的计数值，这个接口需要应用层实现：

uint32_t app_get_cur_rtc_cnt(void )
{
   return RTC计数器;
}
*/
extern uint32_t app_get_cur_rtc_cnt(void );

#define RTC_GET_CNT() app_get_cur_rtc_cnt() 

//////////////////////////////////////////

typedef struct
{
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
}datetime_t;


//datetime模块初始化
void app_datetime_init(void );

//datetime设置
void app_datetime_set(datetime_t *dt);

//获得当前datetime
void app_datetime_get(datetime_t *dt);

/*
获得当天对应的星期，结果保存在pwk中

注意：
0 - 星期一
6 - 星期天
*/
void app_datetime_get_week(uint8_t *pwk);

//获得指定日期对应的星期
void app_datetime_get_specify_date_week(datetime_t *dt, uint8_t *pwk);

//把UTC时间转化为北京时间，北京时区为8
void app_datetime_UTC_to_beijing(datetime_t *utc_time, datetime_t *bj_time, int16_t time_zone);

/*
时钟同步函数。这个函数需要间隔调用，调用间隔最大值由RTC所用的晶振决定

以32768Hz为例，RTC计数器每隔1/32768(大约等于30.5us)累加一次。由于RTC
计数周期为0xffffff，因此RTC的溢出周期是8.5分钟。只要在溢出周期到达前调
用该接口同步一次RTC，就可以确保时间的准确性。

因此，在外部晶振使用32768时，app_datetime_synch()的调用间隔<8.5min。

一般而言，如果你需要显示时钟的秒，那么这个函数每隔1s调用一次是合适的，
如果不需要显示s，每隔1分钟之内调用一次是合适的
*/
void app_datetime_synch(void );

//获得datetime指定的日期前一天的时间并保存在dt_pre中
void app_datetime_get_day_before_date(datetime_t *datetime,datetime_t *dt_pre);

//获得datetime指定的日期后一天的时间并保存在dt_beh中
void app_datetime_get_day_behind_date(datetime_t *datetime, datetime_t *dt_beh);

//获得dt_cur指定日期前time_min分钟的时间并保存在p中。注意，time_min <= 24*60(分钟)
void app_datetime_get_pre_date_time(datetime_t *dt_cur,datetime_t *p,uint16_t time_min);

//获得dt_cur指定日期后time_min分钟的时间并保存在p中。注意，time_min < 24*60(分钟)
void app_datetime_get_behind_date_time(datetime_t *dt_cur, datetime_t *p,uint16_t time_min);

/*
例子：
1、在程序开始时，初始化datetime
app_datetime_init()

2、设置当前时间为2021.8.1/00:00:00
datetime dt;

dt.year = 21;
dt.month = 8;
dt.day = 1;
dt.hour = 0;
dt.min = 0;
dt.sec = 0;
app_datetime_set(&dt);

3、获得当前时间
datetime dt;

app_datetime_get(&dt)

4、获得当前时间对应的星期
uint8_t week;

app_datetime_get_week(&week);

5、假设当前时间是2021.8.1/02:00:00，你要获得前一天对应的时间
datetime dt,dt1;

dt.year = 21;
dt.month = 8;
dt.day = 1;
dt.hour = 2;
dt.min = 0;
dt.sec = 0;

app_datetime_get_day_before_date(&dt,&dt1);
dt1.year = 21;
dt1.month = 7;
dt1.day = 31;
dt1.hour = 2;
dt1.min = 0;
dt1.sec = 0;

6、假设当前时间是2021.8.1/02:00:00，你要获取该时间点前一个小时的时间
datetime dt,dt1;

dt.year = 21;
dt.month = 8;
dt.day = 1;
dt.hour = 2;
dt.min = 0;
dt.sec = 0;

app_datetime_get_pre_date_time(&dt,&dt1,60);
dt1.year = 21;
dt1.month = 8;
dt1.day = 1;
dt1.hour = 1;
dt1.min = 0;
dt1.sec = 0;

7、app_datetime_get_behind_date_time()的使用方法同6
*/


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//下面一部分是计时器

//所谓计时器，是从00（hour）:00（min）:00（sec）:000（ms）处开始计时，hour是(SEC_CHR_HOUR_MAX+1)进制，
//min、sec都是60进制，ms则是1000进制
typedef struct
{
    uint16_t ms;
	uint8_t min;
	uint8_t hour;
	uint8_t sec;
}sec_chr_t;

//计数器的最大小时计数值，这里是99小时
#define SEC_CHR_HOUR_MAX        99

//计时器初始化，他的目的是把ms、min、hour、sec清0
void app_datetime_sec_chr_init(void );

//获得当前的计时器时间
void app_datetime_get_sec_chr(sec_chr_t *psec);

//设置计时起始时间
void app_datetime_set_sec_chr(sec_chr_t *psec);

//同步一次RTC。这个接口的作用在于当计时器从暂停恢复到运行状态时，由于RTC计数器没有暂停，所以重启该计数器
//时，需要先同步一次RTC，避免出现计时器计数错误
void app_datetime_syn_rtc(void );

//原理同app_datetime_synch();
void app_datetime_sec_chr_syn(void );


/*
例子：
1、初始化计时器，时、分、秒、毫秒为0
app_datetime_sec_chr_init();

2、如果暂停，调用下面接口同步RTC，以便下次启动从此刻开始计时
app_datetime_syn_rtc();

3、同步秒表
注意，如果想计时器暂停，则不能调用这个时间同步接口。如果想让计时器从暂停到恢复，则该接口必须调用
app_datetime_sec_chr_syn();

4、获得秒表值
app_datetime_get_sec_chr();

5、设置秒表值
sec_chr_t temp;
temp.hour = 1;
temp.min = 2;
temp.sec = 3;
temp.ms = 0;
app_datetime_set_sec_chr(&temp);

*/



#endif