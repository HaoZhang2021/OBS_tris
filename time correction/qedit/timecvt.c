#ifndef lint
static char sccsid[] = "$Id: timecvt.c,v 1.2 1996/10/23 17:15:19 doug Exp $ ";
#endif

#include "ptime.h"

static int      days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31};

#define mod(a,b)	a - ((int)(a/b)) * b

void
passcal_etoh(t, epoch)
/* fill the time structure *t with values
   equivalent to epochal time epoch    */
    register struct ptime *t;
    double          epoch;
{
    int             diy;
    double          secleft;
    long            days;
    int             passcal_isleap();
    void            passcal_month_day();

    days = (long) ((double) epoch / (double) 86400.);
    secleft = mod(epoch, (double) 86400.0);  
    t->hr = t->mn = 0;
    t->sec = 0.;

    if (secleft) {		/* compute hours minutes seconds */
	if (secleft < 0) {	/* before 1970 */
	    days--;		/* subtract a day */
	    secleft += 86400.;	/* add a day */
	}
	t->hr = (int) (secleft / 3600.);
	secleft = mod(secleft, 3600.0);
	t->mn = (int) (secleft / 60.);
	t->sec = mod(secleft, 60.0);
    }

    if (days >= 0) {
	for (t->yr = 1970;; t->yr++) {
	    diy = passcal_isleap(t->yr) ? 366 : 365;
	    if (days < diy)
		break;
	    days -= diy;
	}
    }
    else {
	for (t->yr = 1969;; t->yr--) {
	    diy = passcal_isleap(t->yr) ? 366 : 365;
	    days += diy;
	    if (days >= 0)
		break;
	}
    }
    /* there was a round-off problem */
    if (t->hr == 23 && t->mn == 59 && t->sec == 60) {  
      t->sec = 0.0;
      t->mn = 0;
      t->hr = 0;
      days++;
    }

    days++;
    passcal_month_day(t, (int) days);
    return;
}

void
passcal_month_day(t, jul_day)
/* set month and day fields of time structure *t
   given the julian day jul_day               */
    register struct ptime *t;
    int             jul_day;
{
    int             i, dim, leap;
    int             passcal_isleap();

    leap = passcal_isleap(t->yr);
    t->day = jul_day;
    for (i = 0; i < 12; i++) {
	dim = days_in_month[i];
	if (leap && (i == 1))
	    dim++;
	if (t->day <= dim)
	    break;
	t->day -= dim;
    }
    t->mo = i + 1;
    return;
}

int
julian(t)
/* returns the julian day represented by *t */
    register struct ptime *t;
{
    int             i, j, inc;

    j = 0;
    for (i = 0; i < t->mo - 1; i++) {
	inc = days_in_month[i];
	if ((i == 1) && passcal_isleap(t->yr))
	    inc++;
	j += inc;
    }
    j += t->day;
    return (j);
}

long
passcal_htoe(t)
/* calculate epochal equivalent of time t */
    register struct ptime *t;
{
    long            i, days;
    long            epoch;

    days = 0;
    if (t->yr > 1970) {
	for (i = 1970; i < t->yr; i++) {
	    days += 365;
	    if (passcal_isleap(i))
		days++;
	}
    }
    else if (t->yr < 1970) {
	for (i = t->yr; i < 1970; i++) {
	    days -= 365;
	    if (passcal_isleap(i))
		days--;
	}
    }
    days += (long) (julian(t) - 1);
    epoch = (((long) days) * 86400.);
    epoch += (long) ((t->hr * 3600.) + (t->mn * 60.) + (t->sec));
    return (epoch);
}

passcal_isleap(year)
/* returns true if leap year, false otherwise */
    int             year;
{
    return (year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
}
