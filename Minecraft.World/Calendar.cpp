#include "stdafx.h"
#include "Calendar.h"
#include <time.h>

unsigned int Calendar::GetDayOfMonth()
{
	time_t t = time(0);
	struct tm *now = localtime(&t);

	return now->tm_mday;
}

unsigned int Calendar::GetMonth()
{
	time_t t = time(0);
	struct tm *now = localtime(&t);

	return now->tm_mon;
}