/*	$Id: get_rate.h,v 1.3 2005/06/29 20:09:49 doug Exp $ 	*/

#ifndef GET_RATE
#define GET_RATE
#include <time.h>

/*  get_rate.h file 
 * 
 * Version 95.069, using additional comment member of struct CLOCK_RATE
 */

#define NULL_PTR   (struct CLOCK_RATE *) NULL

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* define the event types as enumerated in clockfix   */

#define  NO_EVENT		0
#define  START_STOP		1
#define  PHASE_PHASE	2
#define  PHASE_RESET	3
#define  PHASE_UNLOCK	4
#define  PHASE_STOP		5
#define  RESET_PHASE	6
#define  RESET_RESET	7
#define  RESET_UNLOCK	8
#define  UNLOCK_PHASE	9
#define  UNLOCK_STOP	10
#define  STOP_START		11
#define  START_PHASE	12
#define  PHASE_START	13
#define  UNLOCK_START	14
#define  RESET_START	15
#define  RESET_STOP		16

/* declare the Correction structure */
typedef struct CLOCK_RATE {
    double               epoch;		/* epoch seconds */
    int                  inst;		/* das number */
    double               offset;	/* y offset in secs */
    double               slope;		/* slope of correction */
    int                  type;		/* one of the clock event types above */
    int			 line_num; 	/* line number of file */
    int			 modified; 	/* bool, TRUE if offset/slope edited */
    char		 *comment;	/* for tracking the history of edits */
    struct CLOCK_RATE  *next;
} CLOCKRATE;

#ifndef EXTERN
#ifndef MAIN
#define EXTERN extern
#else
#define EXTERN 
#endif	/* MAIN */
#endif	/* EXTERN */

EXTERN CLOCKRATE *last_rate;


/* declare Clock Event structure */
struct  CLK_EVNT {
            int        n_stps;
            double     stops;
            int        n_strt;
            double     starts;
} CLK_EVNT;

CLOCKRATE  *get_rate_list(), *get_rate_entry(), *LastRate(), *get_rate_list_fptr();


#endif  /*  GET_RATE  */
