// TimeSupport.h

#ifndef TIME_SUPPORT_H
#define TIME_SUPPORT_H

#include <SupportDefs.h>

bool	is_leap_year(int year);
void	get_date_for_time(bigtime_t time, int& year, int& month, int& day);
void	get_date_for_time(bigtime_t time, int& year, int& month, int& day,
						  int& hours, int& mins, int& secs, int& micros);
void	print_time(bigtime_t time);

#endif	// TIME_SUPPORT_H
