#include "app_datetime.h"


static sec_chr_t sec_chr;
static uint32_t  sec_chr_rtc_pre;
static datetime_t datetime;

/////////////////////////////////////////////////////////////////////////

void app_rtc_cnt_diff(uint32_t *pdiff)
{
  static uint32_t  time_old = 0;
  uint32_t time_x,time_new;
	
  time_new = RTC_GET_CNT();
		 
  if(time_new >= time_old)
  {
	  time_x = time_new - time_old;
  }
  else
  {
	  time_x = RTC_CNT_REG_MAX - time_old + time_new;
  }
	 
  time_old = time_new;
	 
  *pdiff = time_x;
}

void app_datetime_updata(uint16_t sec)
{
  uint8_t temp;
  uint16_t year;

  datetime.seconds += sec;	
  if(datetime.seconds > 59)
  {
    temp = datetime.seconds / 60;
    datetime.seconds = datetime.seconds - temp * 60;
    datetime.minutes += temp;	
  }
  if(datetime.minutes > 59)
  {
	datetime.minutes = 0;
    datetime.hour++;	
  }
  if(datetime.hour > 23)
  {
 	datetime.hour = 0;
	datetime.day++;
  }
  
  switch(datetime.month)
  {
	 case 2:
        year = datetime.year + 2000;
	    if(((year % 400) == 0) || (((year % 4) == 0) && ((year % 100) != 0)))
		{
		  if(datetime.day > 29)
		  {
			  datetime.day = 1;
			  datetime.month++;
		  }
		}
		else
	    {
		  if(datetime.day > 28)
		  {
			  datetime.day = 1;
			  datetime.month++;
		  }
		}
	  break;
				
	case 4:
	case 6:
	case 9:
	case 11:
		if(datetime.day > 30)
		{
		    datetime.day = 1;	
		    datetime.month++;
		}
	  break;
		
	default:
		if(datetime.day > 31)
		{
		    datetime.day = 1;
            datetime.month++;		  
		}
		break;
  }
	
  if(datetime.month > 12)
  {
	  datetime.month = 1; 
	  datetime.year++;
  }
  if(datetime.year > 100)
  {
	  datetime.year = 99; 
  }
}

void app_datetime_get_day_before_date(datetime_t *datetime,datetime_t *dt_pre)
{
	uint16_t year;
	
	if (datetime->month == 3 && datetime->day == 1)
	{
		dt_pre->year = datetime->year;
		dt_pre->month = 2;
		year = datetime->year + 2000;
		if(((year % 400) == 0) || (((year % 4) == 0) && ((year % 100) != 0)))
		{
			dt_pre->day = 29;
		}
		else
		{
			dt_pre->day = 28;
		}
	}
	else if (datetime->month == 1 && datetime->day == 1)
	{
		dt_pre->year = datetime->year - 1;
		dt_pre->month = 12;
		dt_pre->day = 31;
	}
	else if (datetime->day == 1)
	{
		dt_pre->month = datetime->month - 1;
		dt_pre->year = datetime->year;
		
		if(dt_pre->month == 1 || dt_pre->month == 3 || dt_pre->month == 5 || \
			dt_pre->month == 7 || dt_pre->month == 8 || dt_pre->month == 10 || \
		  dt_pre->month == 12)
		{
			dt_pre->day = 31;
		}
		else
		{
			dt_pre->day = 30;
		}
	}
	else
	{
		dt_pre->day = datetime->day - 1;
		dt_pre->month = datetime->month;
		dt_pre->year = datetime->year;
	}

	dt_pre->hour = datetime->hour;
	dt_pre->minutes = datetime->minutes;
	dt_pre->seconds = datetime->seconds;
}

void app_datetime_get_day_behind_date(datetime_t *datetime, datetime_t *dt_beh)
{
	if (datetime->month == 12)
	{
		if (datetime->day == 31)
		{
			dt_beh->year = datetime->year + 1;
			dt_beh->month = 1;
			dt_beh->day = 1;
		}
		else
		{
			dt_beh->year = datetime->year;
			dt_beh->month = datetime->month;
			dt_beh->day = datetime->day + 1;
		}
	}
	else if (datetime->month == 2)
	{
		uint16_t year;
		uint8_t leap = 0;

		year = datetime->year;
		dt_beh->year = datetime->year;
		dt_beh->month = datetime->month;
		if (((year % 400) == 0) || (((year % 4) == 0) && ((year % 100) != 0)))
		{
			leap = 1;
		}
		else
		{
			leap = 0;
		}

		if (leap == 1)
		{
			if (datetime->day == 29)
			{
				dt_beh->month = 3;
				dt_beh->day = 1;
			}
			else
			{
				dt_beh->day = datetime->day + 1;
			}
		}
		else
		{
			if (datetime->day == 28)
			{
				dt_beh->month = 3;
				dt_beh->day = 1;
			}
			else
			{
				dt_beh->day = datetime->day + 1;
			}
		}
	}
	else if (datetime->month == 1 || datetime->month == 3 || datetime->month == 5 || \
		     datetime->month == 7 || datetime->month == 8 || datetime->month == 10)
	{
		dt_beh->year = datetime->year;
		if (datetime->day == 31)
		{
			dt_beh->month = datetime->month + 1;
			dt_beh->day = 1;
		}
		else
		{
			dt_beh->month = datetime->month;
			dt_beh->day = datetime->day + 1;
		}
	}
	else
	{
		dt_beh->year = datetime->year;
		if (datetime->day == 30)
		{
			dt_beh->month = datetime->month + 1;
			dt_beh->day = 1;
		}
		else
		{
			dt_beh->month = datetime->month;
			dt_beh->day = datetime->day + 1;
		}
	}

	dt_beh->hour = datetime->hour;
	dt_beh->minutes = datetime->minutes;
	dt_beh->seconds = datetime->seconds;
}


//time_min <= 24*60(min)
void app_datetime_get_pre_date_time(datetime_t *dt_cur, datetime_t *p,uint16_t time_min)
{
	uint16_t total_min;
	uint16_t delta;
    datetime_t dt_pre;
	
	total_min = dt_cur->hour*(uint16_t)60 + dt_cur->minutes;
	if(total_min >= time_min)
	{
		delta = total_min - time_min;
		p->hour = delta / 60;
		p->minutes = delta % 60;
		p->day = dt_cur->day;
		p->month = dt_cur->month;
		p->year = dt_cur->year;
	}
	else
	{
		app_datetime_get_day_before_date(dt_cur,&dt_pre);
		
		delta = time_min - total_min;
		delta = (uint16_t)24 * 60 - delta;
		
		p->hour = delta / 60;
		p->minutes = delta % 60;
		p->day = dt_pre.day;
		p->month = dt_pre.month;
		p->year = dt_pre.year;
	}
	
	p->seconds = dt_cur->seconds;
}

//time_min <= 24*60(min)
void app_datetime_get_behind_date_time(datetime_t *dt_cur, datetime_t *p, uint16_t time_min)
{
	uint16_t total_min;
	uint16_t delta;
	uint8_t h, m;
	datetime_t dt_tmp;

	dt_tmp.year = dt_cur->year;
	dt_tmp.month = dt_cur->month;
	dt_tmp.day = dt_cur->seconds;
	dt_tmp.hour = dt_cur->hour;
	dt_tmp.minutes = dt_cur->minutes;
	dt_tmp.seconds = dt_cur->seconds;

	total_min = dt_tmp.hour * 60 + dt_tmp.minutes;
	total_min += time_min;

	if (total_min >= 24*60)
	{
		app_datetime_get_day_behind_date(dt_cur,&dt_tmp);

		delta = total_min - 24 * 60;
		h = delta / 60;
		m = delta % 60;
	}
	else
	{
		h = total_min / 60;
		m = total_min % 60;
	}

	p->day = dt_tmp.day;
	p->month = dt_tmp.month;
	p->year = dt_tmp.year;
	p->seconds = dt_tmp.seconds;
	p->hour = h;
	p->minutes = m;
}

void app_datetime_synch(void )
{
    uint32_t diff;
    uint32_t sec_cnt;
    static uint32_t total_diff = 0; 
  
    app_rtc_cnt_diff(&diff);
  
    total_diff += diff;
    sec_cnt = total_diff / RTC_FREQUEN;
  
	if(sec_cnt != 0)
	{
	  total_diff = total_diff - sec_cnt*RTC_FREQUEN;
	}
	
	app_datetime_updata(sec_cnt);
}

void app_datetime_init(void )
{
  uint32_t diff;
	
  datetime.seconds = 00;
  datetime.minutes = 00;
  datetime.hour = 2;
  datetime.day = 30;
  datetime.month = 12;
  datetime.year = 21;
	
  app_rtc_cnt_diff(&diff);
}

void app_datetime_set(datetime_t *dt)
{
  datetime.seconds = dt->seconds;
  datetime.minutes = dt->minutes;
  datetime.hour = dt->hour;
  datetime.day = dt->day;
  datetime.month = dt->month;
  datetime.year = dt->year;
}

void app_datetime_get(datetime_t *dt)
{
  dt->seconds = datetime.seconds;
  dt->minutes = datetime.minutes;
  dt->hour = datetime.hour;
  dt->day = datetime.day;
  dt->month = datetime.month;
  dt->year = datetime.year;
}

int app_datetime_week_cal(datetime_t *dt)
{
	int week = 0;
	int y, m, d;
	
	y = dt->year+2000;
	m = dt->month;
	d = dt->day;
		
    if(m==1||m==2) 
	{
        m+=12;
        y--;
    }
	 
    week=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
 
	return week;
}

void app_datetime_f24_to_f12(datetime_t *f_24, datetime_t *f_12)
{
	f_12->year = f_24->year;
	f_12->month = f_24->month;
	f_12->day = f_24->day;
	if (f_24->hour >= 12)
	{
		f_12->hour = f_24->hour - 12;
	}
	else
	{
		f_12->hour = f_24->hour;
	}
	f_12->minutes = f_24->minutes;
	f_12->seconds = f_24->seconds;
}

void app_datetime_get_specify_date_week(datetime_t *dt, uint8_t *pwk)
{
	*pwk = app_datetime_week_cal(dt);
}

void app_datetime_UTC_to_beijing(datetime_t *utc_time, datetime_t *bj_time, int16_t time_zone)
{
	uint8_t days = 0;
	uint16_t year;

    bj_time->day = utc_time->day;
    bj_time->month = utc_time->month;
    bj_time->year = utc_time->year;
    bj_time->hour = utc_time->hour;
    bj_time->minutes = utc_time->minutes;
    bj_time->seconds = utc_time->seconds;
	
	if (bj_time->month == 1 || bj_time->month == 3 || bj_time->month == 5 || \
		  bj_time->month == 7 || bj_time->month == 8 || bj_time->month == 10 || bj_time->month == 12)
	{
		days = 31;
	}
	else if (bj_time->month == 4 || bj_time->month == 6 || bj_time->month == 9 || bj_time->month == 11)
	{
		days = 30;
	}
	else if (bj_time->month == 2)
	{
		year = bj_time->year + 2000;
		if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0))) 
		{
			days = 29;
		}
		else
		{
			days = 28;
		}
	}
	
	bj_time->hour += time_zone;
	if (bj_time->hour >= 24)         
	{
		bj_time->hour -= 24;
		bj_time->day++;
		if (bj_time->day > days)     
		{
			bj_time->day = 1;
			bj_time->month++;
			if (bj_time->month > 12)  
			{
				bj_time->year++;
			}
		}
	}
}

void app_datetime_get_week(uint8_t *pwk)
{
  *pwk = app_datetime_week_cal(&datetime);
}

////////////////////////////////////////////////
//second chronograph
static uint32_t sec_chr_total_diff = 0; 
uint32_t app_datetime_sec_chr_get_rtc_diff(void )
{
	uint32_t rtc_cur,rtc_x;

	rtc_cur = app_get_cur_rtc_cnt();
	if(rtc_cur >= sec_chr_rtc_pre)
    {
	  rtc_x = rtc_cur - sec_chr_rtc_pre;
    }
    else
    {
	  rtc_x = RTC_CNT_REG_MAX - sec_chr_rtc_pre + rtc_cur;
    }
	 
    sec_chr_rtc_pre = rtc_cur;
	
	return rtc_x;
}

void app_datetime_syn_rtc(void )
{
	sec_chr_rtc_pre = app_get_cur_rtc_cnt();
}

void app_datetime_sec_chr_init(void )
{
	sec_chr.sec = 0;
	sec_chr.ms = 0;
	sec_chr.hour = 0;
	sec_chr.min = 0;
	sec_chr_total_diff = 0;
	sec_chr_rtc_pre = app_get_cur_rtc_cnt();
}

void app_datetime_get_sec_chr(sec_chr_t *psec)
{
	psec->hour = sec_chr.hour;
	psec->min = sec_chr.min;
	psec->ms = sec_chr.ms;
	psec->sec = sec_chr.sec;
}

void app_datetime_set_sec_chr(sec_chr_t *psec)
{
	sec_chr.hour = psec->hour;
	sec_chr.min = psec->min;
	sec_chr.sec = psec->sec;
	sec_chr.ms = psec->ms;
}

void app_datetime_sec_chr_syn(void )
{
	uint32_t rtc_diff;
	uint8_t time_ms;
	 
	rtc_diff = app_datetime_sec_chr_get_rtc_diff();
	sec_chr_total_diff += rtc_diff;
	time_ms = sec_chr_total_diff / (RTC_FREQUEN / 10);
	
	if(time_ms != 0)
	{
		sec_chr_total_diff = sec_chr_total_diff - time_ms * (RTC_FREQUEN / 10);
	}
	
	sec_chr.ms += time_ms;
	if(sec_chr.ms > 9)
	{
		uint8_t temp;
		
		temp = sec_chr.ms / 10;
	  	sec_chr.ms = sec_chr.ms - temp * 10;
    	sec_chr.sec += temp;	
	}
	if(sec_chr.sec > 59)
	{
		sec_chr.sec = 0;
		sec_chr.min++;
	}
	if(sec_chr.min > 59)
	{
		sec_chr.min = 0;
		sec_chr.hour++;
	}
	if(sec_chr.hour > SEC_CHR_HOUR_MAX)
	{
		sec_chr.hour = SEC_CHR_HOUR_MAX;
	}
}
