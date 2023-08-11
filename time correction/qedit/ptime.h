/*	$Id: ptime.h,v 1.2 1996/10/23 17:15:13 doug Exp $ 	*/

/* this structure is used by PQL in conjunction with
	the timecvt.c time conversion functions */
typedef struct ptime {
	short yr;	/* year		*/
	short mo;	/* month	*/
	short day;	/* day		*/
	short hr;	/* hour		*/
	short mn;	/* minute	*/
	float sec;	/* second	*/
} PTIME;
