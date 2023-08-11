#ifndef lint
static char sccsid[] = "$Id: epoch.c,v 1.2 1996/10/23 17:15:21 doug Exp $ ";
#endif

#include "epoch.h"

void epoch2gmt (time, epoch, leap_sec)
Time	*time;
Epoch	*epoch;
int	leap_sec;
{
	long epoch_year=1970;
	long days, years_since, leap_years;
	long sec_left;

	epoch->sec = epoch->sec - leap_sec;
	days = epoch->sec / 86400;

	leap_years = ((days + 365) / 1461);
	years_since = ((days - leap_years) / 365);
	time->year = years_since + epoch_year;

	/* need to add one, since range of julian day (1-365/366)	*/
	time->day = (days - years_since*365 - leap_years) + 1;

	sec_left = epoch->sec % 86400;

	time->hour = sec_left / 3600;
	sec_left = epoch->sec - time->hour*3600;
	sec_left = sec_left % 3600;

	time->min = sec_left / 60;
	sec_left = epoch->sec - time->min*60;
	time->sec = sec_left % 60;

	time->msec = epoch->usec / 1000;
	time->usec = epoch->usec % 1000;
}

void gmt2epoch (epoch, time, leap_sec)
Epoch	*epoch;
Time	*time;
int	leap_sec;
{
	long epoch_year=1970;
	long years_since, leap_years, days, sec;

	years_since = time->year - epoch_year;

	leap_years = (years_since + 1) / 4;

	/* need to subtract one, since range of julian day (1-365/366)	*/
	days = years_since*365 + time->day + leap_years - 1;

	sec = days*86400 + time->hour*3600 + time->min*60 + time->sec;
	epoch->sec = sec + leap_sec;

	epoch->usec = time->msec*1000 +  time->usec;
}

Epoch subtract_epoch (epoch2, epoch1)
Epoch *epoch2, *epoch1;
{
	Epoch epoch;
 
	if (epoch2->usec < epoch1->usec) {
		epoch.usec = (epoch2->usec + 1000000) - epoch1->usec;
		epoch.sec = (epoch2->sec - 1) - epoch1->sec;
	}
	else {
		epoch.usec = epoch2->usec - epoch1->usec;
		epoch.sec = epoch2->sec - epoch1->sec;
	}
 
	return (epoch);
}

int epoch_compare (epoch1, epoch2)
Epoch *epoch1, *epoch2;
{
	if (epoch1->sec > epoch2->sec)
		return(1);
	else if (epoch1->sec < epoch2->sec)
		return(-1);
	else if (epoch1->usec > epoch2->usec)
		return(1);
	else if (epoch1->usec < epoch2->usec)
		return(-1);
	else
		return(0);
}
Epoch add_epoch (epoch1, epoch2)
Epoch *epoch1, *epoch2;
{
        Epoch epoch;
        int usec;

        usec = epoch1->usec + epoch2->usec;
        epoch.usec = usec % 1000000;
        epoch.sec = epoch1->sec + epoch2->sec + (usec / 1000000);

        if ((epoch.sec > 0) && (epoch.usec < 0)) {
                epoch.usec += 1000000;
                epoch.sec--;
        }
        else if ((epoch.sec < 0) && (epoch.usec > 0)) {
                epoch.usec -= 1000000;
                epoch.sec++;
        }

        return (epoch);
}

double epoch2double (epoch)
Epoch *epoch;
{
        double depoch;
        double fraction;

        fraction = ( (double) epoch->usec) / 1000000.0;
        depoch = ((double) epoch->sec) + fraction;

        return (depoch);
}

