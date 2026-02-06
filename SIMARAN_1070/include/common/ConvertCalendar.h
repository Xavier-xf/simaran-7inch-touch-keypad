/**
*
* @Name : ConvertCalendar.h
* @Version : 1.0
* @Programmer : Max
* @Date : 2019-03-31
* @Released under : https://github.com/BaseMax/ConvertCalendar/blob/master/LICENSE
* @Repository : https://github.com/BaseMax/ConvertCalendar
*
**/
#ifndef CONVERT_CALENDAR
#define CONVERT_CALENDAR

struct date{
	// We can use the uint8_t type.
	int year;
	int month;
	int day;
	int wday;
};

struct date gregorian2jalali(struct date input);
struct date jalali2gregorian(struct date input);

#endif
