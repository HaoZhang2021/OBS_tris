/*	$Id: epoch.h,v 1.2 1996/10/23 17:15:15 doug Exp $ 	*/

/* Public header file for epoch.c	*/

#ifndef EPOCH_H
#define EPOCH_H

typedef struct time_type {
	short	year;
	short	day;
	short	hour;
	short	min;
	short	sec;
	short	msec;
	short	usec;
} Time;

typedef struct epoch_type {
	long	sec;
	long	usec;
} Epoch;

#ifdef ANSI_C
void epoch2gmt (TIME *, EPOCH *, int);
void gmt2epoch (EPOCH *, TIME *, int);
Epoch add_epoch (Epoch *epoch1, Epoch *epoch2);
Epoch subtract_epoch (Epoch *, Epoch *);
double epoch2double (Epoch *epoch);
int epoch_compare (Epoch *, Epoch *);
#else
Epoch add_epoch ();
void epoch2gmt();
void gmt2epoch ();
double epoch2double ();
Epoch subtract_epoch();
int epoch_compare();
#endif

#endif /* EPOCH_H	*/
