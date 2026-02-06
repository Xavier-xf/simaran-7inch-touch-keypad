#include "user_time.h"
#include <sys/time.h>

/***
** 日期: 2022-05-09 17:29
** 作者: leo.liu
** 函数作用：获取当前日期是星期几
** 返回参数说明：
***/
int user_cur_time_is_week(struct tm *date)
{

	int m = date->tm_mon;
	int y = date->tm_year;
	int d = date->tm_mday;
	if (m == 1 || m == 2)
	{
		m += 12;
		y--;
	}
	return (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7 + 1;
}




/***
** 日期: 2022-05-09 17:32
** 作者: leo.liu
** 函数作用：获取西方日历当前月份下最后一天日期
** 返回参数说明：
***/
int user_western_calendar_month_last_day(unsigned int year, unsigned char mon)
{
	if (mon == 2)
	{
		if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400) == 0)
		{
			return 29;
		}
		else
		{
			return 28;
		}
	}
	else
	{
		switch ((mon))
		{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return 31;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
			break;
		default:
			break;
		}
	}
	return 0;
}
/***
** 日期: 2022-05-09 17:32
** 作者: leo.liu
** 函数作用：获取波斯日历当前月份下最后一天日期
** 返回参数说明：
***/
int user_persian_calendar_month_last_day(unsigned int year, unsigned char mon)
{
	if (mon == 12)
	{
		if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400) == 0)
		{
			return 30;
		}
		else
		{
			return 29;
		}
	}
	else
	{
		switch ((mon))
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			return 31;
			break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			return 30;
			break;
		default:
			break;
		}
	}
	return 0;
}
/***
** 日期: 2022-04-26 10:58
** 作者: leo.liu
** 函数作用：将RTC时间同步到系统时间
** 返回参数说明：
***/
void user_time_sync(void)
{
	system("hwclock -s");
}

/***
** 日期: 2022-04-26 10:49
** 作者: leo.liu
** 函数作用：获取用户时间
** 返回参数说明：
***/
bool user_time_read(struct tm *tm)
{
	time_t t = time(NULL);
	tzset();
	localtime_r(&t, tm);
	tm->tm_year += 1900;
	tm->tm_mon += 1;
	tm->tm_wday = user_cur_time_is_week(tm);
	return true;
}
/***
** 日期: 2022-04-26 10:57
** 作者: leo.liu
** 函数作用：设置系统时间并同步到RTC
** 返回参数说明：
***/
bool user_time_set(struct tm *tm)
{
	char string[64] = {0};
	sprintf(string, "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	system(string);
	/***** 将系统时间与RTC同步 ******/
	system("hwclock -w");

	return true;
}
/***
** 日期: 2022-04-26 10:33
** 作者: leo.liu
** 函数作用：用户时间初始化
** 返回参数说明：
***/
bool user_time_init(void)
{
	// user_time_sync();
	struct tm tm;
	user_time_read(&tm);
	if(tm.tm_year < 2022)
	{
		tm.tm_year = 2022;
		tm.tm_mon = 1;
		tm.tm_mday = 1;
		tm.tm_hour = 0;
		tm.tm_min = 0;
		tm.tm_sec = 0;
		user_time_set(&tm);
	}
	user_time_sync();
	return true;
}
/***
** 日期: 2022-04-26 11:02
** 作者: leo.liu
** 函数作用：获取当前时间戳
** 返回参数说明：
***/
unsigned long long user_timestamp_get(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
/***
**   日期:2022-06-09 11:45:51
**   作者: leo.liu
**   函数作用：将tm转换成sec
**   参数说明:
***/
int user_tm_conver_sec(struct tm *info)
{
	info->tm_year -= 1900;
	info->tm_mon -= 1;
	info->tm_mday -= 1;
	info->tm_isdst = -1;
	return mktime(info);
}



static int DAYS = 24*3600;
static int FOURYEARS = 365*3+366;
static int norMoth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int leapMoth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


static void getHourMinSec(int nSecond)
{
    int nHour = nSecond/3600;
    int nMin = (nSecond%3600)/60;
    int nSec = (nSecond%3600)%60;
    printf("%d:%d:%d\n", nHour+8, nMin, nSec);
}


static void getMothAndDay(bool bLeapYear, int nDays, int *nMoth, int *nDay)
{
    int i = 0;
    int nTmp = 0;
    int *pMoth = bLeapYear?leapMoth:norMoth;


    for (i=0; i<12; i++)
    {
        nTmp = nDays-pMoth[i];
        if (nTmp <= 0)
        {
            *nMoth = i+1;
            if (nTmp == 0)
            {
                *nDay = pMoth[i];
            }
            else
            {
                *nDay = nDays;
            }
            break;
        }
        nDays = nTmp;
    }
    return;
}


void print_time(void)
{
	struct timespec time = {0, 0};
	clock_gettime(CLOCK_REALTIME, &time);
	time_t nTime = time.tv_sec;
	
    // time_t nTime = time(NULL);
    int nDays = nTime/DAYS + ((nTime%DAYS)?1:0);
    int nYear4 = nDays/FOURYEARS;
    int nRemain = nDays%FOURYEARS;
    int nDecyear = 1970 + nYear4*4;
    int nDecmoth = 0;
    int nDecday = 0;
    bool bLeapyear = false;


    if (nRemain < 365)
    {
        ;
    }
    else if (nRemain < 365*2)
    {
        nDecyear += 1;
        nRemain -= 365;
    }
    else if (nRemain < 365*3)
    {
        nDecyear += 2;
        nRemain -= 365*2;
    }
    else
    {
        nDecyear += 3;
        nRemain -= 365*3;
        bLeapyear = true;
    }
	printf("*********************************\n");
	getMothAndDay(bLeapyear, nRemain, &nDecmoth, &nDecday);
    printf("%d:%d:%d ", nDecyear, nDecmoth, nDecday);
    getHourMinSec(nTime%DAYS);
    return;
}