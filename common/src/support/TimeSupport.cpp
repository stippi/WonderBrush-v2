// TimeSupport.cpp

#include <cstdio>

#include "TimeSupport.h"

// prototypes
static int get_absolute_day(int year, int month, int day);

// constants
const int kMonthDays[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
const int kCumulatedMonthDays[] = {
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};
const int kRTCDateOrigin = get_absolute_day(1970, 0, 0);
const bigtime_t kRTCTimeOrigin
	= (bigtime_t)kRTCDateOrigin * 24 * 60 * 60 * 1000000;


// is_leap_year
bool
is_leap_year(int year)
{
	return !(year % 4) && ((year % 100) || !(year % 400) && (year % 10000));
}

// get_absolute_day
static
int
get_absolute_day(int year, int month, int day)
{
	year--;
	int days = year * 365 + year / 4 - year / 100 + year / 400 - year / 10000;
	year++;
	days += kCumulatedMonthDays[month] + day;
	if (month > 1 && is_leap_year(year))
		days++;
	return days;
}

// get_date_of_absolute_day
static
void
get_date_of_absolute_day(int days, int& year, int& month, int& day)
{
	// get the year
	year = 0;
	int yearDays = 0;
	do {
		year += (days - yearDays) / 366;
		yearDays = get_absolute_day(year, 0, 0);
	} while (days - yearDays >= 366);
	if (days - yearDays == 365 && !is_leap_year(year)) {
		year++;
		yearDays += 365;
	}
	days -= yearDays;
	// get the month
	month = 0;
	int leapDay = is_leap_year(year) ? 1 : 0;
	for (; month < 11; month++) {
		if (days < kCumulatedMonthDays[month + 1] + (month > 0 ? leapDay : 0))
			break;
	}
	days -= kCumulatedMonthDays[month];
	if (month > 1)
		days -= leapDay;
	// get the day
	day = days;
}


// get_date_for_time
//
// Note: Does not handle dates before the julian -> gregorian calendar switch
// correctly. 
void
get_date_for_time(bigtime_t time, int& year, int& month, int& day)
{
	int hours, mins, secs, micros;
	get_date_for_time(time, year, month, day, hours, mins, secs, micros);
}

// get_date_for_time
//
// Note: Does not handle dates before the julian -> gregorian calendar switch
// correctly. 
void
get_date_for_time(bigtime_t time, int& year, int& month, int& day,
				  int& hours, int& mins, int& secs, int& micros)
{
	int days;
	bool negative = (time < 0);
	if (negative)
		time += kRTCTimeOrigin;
	micros = time % 1000000LL;
	time /= 1000000;
	secs = time % 60;
	time /= 60;
	mins = time % 60;
	time /= 60;
	hours = time % 24;
	time /= 24;
	days = time;
	if (!negative)
		days += kRTCDateOrigin;
	get_date_of_absolute_day(days, year, month, day);
}

// print_time
//
// Note: Does not handle dates before the julian -> gregorian calendar switch
// correctly. 
void
print_time(bigtime_t time)
{
	int year, month, day, hours, mins, secs, micros;
	get_date_for_time(time, year, month, day, hours, mins, secs, micros);
	printf("%d/%d/%d %d:%02d:%02d.%06d",
		   year, month + 1, day + 1, hours, mins, secs, micros);
}
