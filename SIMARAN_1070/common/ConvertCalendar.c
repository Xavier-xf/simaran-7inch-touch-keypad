/**
*
* @Name : ConvertCalendar.c
* @Version : 1.0
* @Programmer : Max
* @Date : 2019-03-31
* @Released under : https://github.com/BaseMax/ConvertCalendar/blob/master/LICENSE
* @Repository : https://github.com/BaseMax/ConvertCalendar
*
**/
#include "ConvertCalendar.h"

long *gregorian_to_jalali(long gy, long gm, long gd, long out[])
{
	long days;
	{
		long gy2 = (gm > 2) ? (gy + 1) : gy;
		long g_d_m[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
		days = 355666 + (365 * gy) + ((int)((gy2 + 3) / 4)) - ((int)((gy2 + 99) / 100)) + ((int)((gy2 + 399) / 400)) + gd + g_d_m[gm - 1];
	}
	long jy = -1595 + (33 * ((int)(days / 12053)));
	days %= 12053;
	jy += 4 * ((int)(days / 1461));
	days %= 1461;
	if (days > 365)
	{
		jy += (int)((days - 1) / 365);
		days = (days - 1) % 365;
	}
	out[0] = jy;
	if (days < 186)
	{
		out[1] /*jm*/ = 1 + (int)(days / 31);
		out[2] /*jd*/ = 1 + (days % 31);
	}
	else
	{
		out[1] /*jm*/ = 7 + (int)((days - 186) / 30);
		out[2] /*jd*/ = 1 + ((days - 186) % 30);
	}
	return out;
}

long *jalali_to_gregorian(long jy, long jm, long jd, long out[])
{
	jy += 1595;
	long days = -355668 + (365 * jy) + (((int)(jy / 33)) * 8) + ((int)(((jy % 33) + 3) / 4)) + jd + ((jm < 7) ? (jm - 1) * 31 : ((jm - 7) * 30) + 186);
	long gy = 400 * ((int)(days / 146097));
	days %= 146097;
	if (days > 36524)
	{
		gy += 100 * ((int)(--days / 36524));
		days %= 36524;
		if (days >= 365)
			days++;
	}
	gy += 4 * ((int)(days / 1461));
	days %= 1461;
	if (days > 365)
	{
		gy += (int)((days - 1) / 365);
		days = (days - 1) % 365;
	}
	long gd = days + 1;
	long gm;
	{
		long sal_a[13] = {0, 31, ((gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		for (gm = 0; gm < 13 && gd > sal_a[gm]; gm++)
			gd -= sal_a[gm];
	}
	out[0] = gy;
	out[1] = gm;
	out[2] = gd;
	return out;
}
/*******************************************************************
 * @brief  : 公历转波斯日历
 * @return  {*}
 * @param {date} input
 *******************************************************************/
struct date gregorian2jalali(struct date input)
{
	struct date result;
	long ymd_out[3] = {0};
	gregorian_to_jalali(input.year, input.month, input.day, ymd_out);
	result.year = ymd_out[0];
	result.month = ymd_out[1];
	result.day = ymd_out[2];
	return result;
}

/*******************************************************************
 * @brief  : 波斯日历转公历
 * @return  {*}
 * @param {date} input
 *******************************************************************/
struct date jalali2gregorian(struct date input)
{
	struct date result;
	long ymd_out[3] = {0};
	jalali_to_gregorian(input.year, input.month, input.day, ymd_out);
	result.year = ymd_out[0];
	result.month = ymd_out[1];
	result.day = ymd_out[2];
	return result;
}
