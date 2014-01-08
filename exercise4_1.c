#include <stdio.h>

//1. 给出年、月、日，计算该日是该年的第 N天。 

struct date_type 
{
	int year;
	int month;
	int day;
};

int is_leap(int year){
	int leap;
	if (year % 100 == 0)
	{
		leap = (year % 400) ? 1: 0;
	}else if (year % 4 == 0) {
		leap = 1;
	}else {
		leap = 0;
	}
	return leap;
}

int count_day(date_type date){
	int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int count = 0;    //need to manually set to 0
	month_days[1] += is_leap(date.year);
	for (int i = 0; i < date.month; ++i)
	{
		count += month_days[i];
	}
	return count;
}

int main(int argc, char const *argv[])
{
	date_type date;
	int count;
	scanf("%d,%d,%d", &date.year, &date.month, &date.day);
	count = count_day(date);
	printf("date.year %d, date.month %d, date.day %d,\n    is the %dth day of the year\n",
			date.year, 	  date.month,    date.day, 			   	    count);
	return 0;
}