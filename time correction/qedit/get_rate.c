#ifndef lint
static char sccsid[] = "$Id: get_rate.c,v 1.3 2004/03/07 00:08:24 doug Exp $ ";
#endif

/*
 * Function get_rate_list( char *rate_file, int start_epoch, int end_epoch)
 * builds a linked list of clockfix format REFTEK clock ratings and returns a
 * pointer to start of struct CLOCK_RATE list.
 * 
 * Parameters: 
 *        char  *filename               - rating file name (output of RefRate)
 *        int    start_epoch, end_epoch - if time range is known,
 * 
 * get_rate_list will include only those times, if unknown, use zeros for
 * both 
 * 
 * NOTE: for multiple DAS rating files, set start_epoch and end_epoch to zeros
 * 
 * Version 95.069, using additional comment member of struct CLOCK_RATE
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "get_rate.h"
#include "ptime.h"
#include "epoch.h"


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_LINE_LEN 256

extern char    *cmdname;
char * progname;

#if __STDC__ || defined(__cplusplus)
CLOCKRATE *
get_rate_list(char *filename, int start_epoch, int end_epoch)
#else
CLOCKRATE *
get_rate_list(filename, start_epoch, end_epoch)
  char           *filename;
  int             start_epoch, end_epoch;
#endif
{
  FILE           *fp;
  CLOCKRATE *get_rate_list_fptr();
  progname = cmdname;
  /* open clock rating file for input  */
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "%s: Error, Unable to open file: %s\n", progname, filename);
	return NULL;
  }
  return get_rate_list_fptr(fp, filename, start_epoch, end_epoch);
}
#if __STDC__ || defined(__cplusplus)
CLOCKRATE *
get_rate_list_fptr(FILE *fp, char *filename, int start_epoch, int end_epoch)
#else
CLOCKRATE *
get_rate_list_fptr(fp, filename, start_epoch, end_epoch)
  char           *filename;
  FILE           *fp;
  int             start_epoch, end_epoch;
#endif
{
  struct CLOCK_RATE *previous, *s_ptr = NULL_PTR, *e_ptr, *n_ptr;
  struct ptime    t;
  double          offset, slope, mod;
  long            passcal_htoe();
  int             err, previous_zero, this_zero, started, num_lines = 0;
  int             line_num, sec, ms, ee = 0, i = 0, start = 0, First_NULL = 0;
  /* new to 95.069 to handle multiple comments at the end of line - jrw */
  int             num_comments, linenum, comment_size, ii;
  void            passcal_month_day();
  char            **line, line_buffer[MAX_LINE_LEN], E_type[16];
  char            **ParseLine();


  if (start_epoch == 0 && end_epoch == 0) {
    end_epoch = INT_MAX;
  }
  /* Check file format */
  line_num = 0;
  while ((fgets(line_buffer, MAX_LINE_LEN, fp)) && (strncmp(line_buffer, "#!PCF", 5) != 0))
    line_num++;
  if ((strncmp(line_buffer, "#!PCF", 5) != 0)) {
    fprintf(stderr, "\n Data file %s not of proper format!\n", filename);
    fprintf(stderr, "ONLY PCF FORMAT ACCEPTED, reformat with clockrate2refrate\n");
    exit(1);
  }
  line_num++;

  /* create null entry for use */
  n_ptr = (struct CLOCK_RATE *) calloc(1, sizeof(struct CLOCK_RATE));
  if (n_ptr == NULL_PTR)
    fprintf(stderr, "%s: Out of memory reading clock rating file: \
      s\n", progname, filename);
  previous = (struct CLOCK_RATE *) calloc(1, sizeof(struct CLOCK_RATE));
  if (previous == NULL_PTR)
    fprintf(stderr, "%s: Out of memory reading clock rating file: \
      s\n", progname, filename);
  previous->epoch = 0;

  previous_zero = FALSE;
  started = FALSE;
  s_ptr = NULL_PTR;

/* new to 95.069 to handle multiple comments at the end of line - jrw */
/* ParseLine breaks line up at "#" and I call each section a comment  */
  /* get line from rate file */
  while (fgets(line_buffer, MAX_LINE_LEN, fp) != NULL) {
    /* Skip past comment lines  */
    if((line = ParseLine(line_buffer, "#")) == NULL) {
      line_num++;
      continue;
    }
    comment_size = num_comments = 0;
    /* count the "comments" on this line */
    while(line[num_comments] != NULL) {
      E_type[0] = '\0'; 
      switch (num_comments) {
      /* Scan the first "comment", the rating info */
      case 0:
	line_num++;
	if ((int) strlen(line[num_comments]) < (int) 54) {
	  fprintf (stderr, "Format problem in rating file, line %d\n", linenum);
	  return ( (struct CLOCK_RATE *) NULL);
	}
	/* Notice the %hd and %le formats, for shorts and doubles */
	sscanf(line[num_comments],
	  "%d %hd:%hd:%hd:%hd:%d:%d %le %le",
	  &n_ptr->inst, &t.yr, &t.day, &t.hr, &t.mn, &sec, &ms,
	  &n_ptr->offset, &n_ptr->slope);
	  passcal_month_day(&t, t.day);
	  t.sec = sec + ms / 1000.;
	  n_ptr->epoch = (double) (passcal_htoe(t) + ms / 1000.);
	  n_ptr->line_num = line_num;
	  n_ptr->modified = FALSE;
	  if (n_ptr->offset == 0.0 && n_ptr->slope == 0.0) {
	    this_zero = TRUE;
	  } else {
	    this_zero = FALSE;
	  }
	break;
      case 1:
	/* Scan the second "comment", the Clock Event String */
	sscanf(line[num_comments], "%s", E_type);
	n_ptr->type = parse_event(E_type);
	break;
      default:
      /* Scan the remaining "comments" */
	comment_size += strlen(line[num_comments]) + 2;
	break;
      }
      num_comments++;
    }

    if (num_comments > 2) {
      /* Allocate and copy comments to structure  */
      n_ptr->comment = (char *) calloc (comment_size, sizeof(char));
      sprintf (n_ptr->comment, "");
      for (ii = 2; ii < num_comments; ii++) {
	strcat (n_ptr->comment, "#");
	strcat (n_ptr->comment, line[ii]);
	/* sprintf (n_ptr->comment, "# %s ", line[ii]); */
      }
    }


    if (!started) {
      /*
       * compare this entry with last line. Is time of this
       * entry after start_epoch?
       */
      if (n_ptr->epoch < start_epoch) {
        /* update previous with this entry */
        previous->epoch = n_ptr->epoch;
        previous->type = n_ptr->type;
        previous->offset = n_ptr->offset;
        previous->slope = n_ptr->slope;
        previous->inst = n_ptr->inst;
        previous->line_num = n_ptr->line_num;
        previous->modified = n_ptr->modified;
        previous->comment = n_ptr->comment;
      } else {
        if (start_epoch == 0) {
          /* save this entry as header of list */
          s_ptr = n_ptr;
        } else {
          /* save last entry as header of list */
          s_ptr = previous;
          s_ptr->next = n_ptr;
        }
        /* set end ptr and begin */
        e_ptr = n_ptr;
        started = TRUE;
        /* create new member */
        n_ptr = (struct CLOCK_RATE *) calloc(1, sizeof(struct CLOCK_RATE));
        if (n_ptr == NULL_PTR)
          fprintf(stderr, "%s: Out of memory reading clock rating file: \
            %s\n", progname, filename);
      }
      continue;
    }
    /* perform checks to save this entry */
    if (!(previous_zero && this_zero &&
          !(n_ptr->type == START_STOP ||
      n_ptr->type == START_PHASE ||
      n_ptr->type == UNLOCK_STOP))) {

      if (n_ptr->epoch < end_epoch) {
        /* link new rate entry to last one */
        e_ptr->next = n_ptr;
        e_ptr = n_ptr;
        /* check for zeros */
        if (this_zero) {
          previous_zero = TRUE;
        } else {
          previous_zero = FALSE;
        }
        /* create new member */
        n_ptr = (struct CLOCK_RATE *) calloc(1, sizeof(struct CLOCK_RATE));
        if (n_ptr == NULL_PTR)
          fprintf(stderr, "%s: Out of memory reading clock rating file: \
              %s\n", progname, filename);
      }
    }
  }  
/* This covers the occasional case of a zero length ratingfile 95.079 jrw */
  if (!started)
    return (struct CLOCK_RATE *) NULL;
  else
    e_ptr->next = NULL_PTR;


  /* DEBUG */
  /*
   * while (s_ptr->next != NULL_PTR)  { fprintf (stderr, "W3 %12.2f\t %
   * e\t% e\n", s_ptr->epoch, s_ptr->offset, s_ptr->slope); s_ptr =
   * s_ptr->next; } exit (1);
   */
  fclose (fp);
  return s_ptr;
}

/******************************************************************/
/*
 * returns the rate entry that immediately preceeds the epoch for a given das
 * from list
 * 
 */

/* add this for speeding up rate entry searches assuming they are sequential */

extern CLOCKRATE * last_rate;

#if __STDC__ || defined(__cplusplus)
CLOCKRATE *
	get_rate_entry(CLOCKRATE *list, double epoch, int das)
#else
CLOCKRATE *
	get_rate_entry(list, epoch, das)
  CLOCKRATE *list;
  double          epoch;
  int             das;
#endif
{
	struct CLOCK_RATE * current, *get_rate_entry_start();
	/*	96.102 - start with last_rate if not NULL  */
	if (last_rate != NULL) {
		if (last_rate->epoch < epoch)  {
			current = get_rate_entry_start(last_rate, epoch, das);
			if (current != NULL) {
				last_rate = current;
				return current;
			}
		}
	}
	last_rate = get_rate_entry_start(list, epoch, das);
	return last_rate;
}

#if __STDC__ || defined(__cplusplus)
CLOCKRATE *
get_rate_entry_start(CLOCKRATE *list, double epoch, int das)
#else
CLOCKRATE *
get_rate_entry_start(list, epoch, das)
  struct CLOCK_RATE *list;
  double          epoch;
  int             das;
#endif
{
CLOCKRATE *current, *previous;

  /* for now do a linear search, and check DAS number!! 
  */

  /* Start with NULL in case  DAS not found or epoch < list->epoch */
  previous = NULL_PTR;

  for (current = list; current != NULL_PTR; current = current->next) {
    /* find first DAS entry in list  */
    if (current->inst != das && das != 0) {
      /* Move through list till DAS is found */
      previous = current;
    } else {
      /* DAS found! */
      if (current->epoch < epoch) {
        /* Move through list till epoch is found */
        previous = current;
        if (current->next == NULL_PTR)
          /*
           * Reached end of list but no epoch
           * big enough
           */
          return previous;
      } else {
        /* Epoch is found!, Return it */
        return previous;
      }
    }
  }
  return NULL_PTR;  /* DAS not found */
}

/******************************************************************/
/* this function returns the type of clockevent as a DEFINE value */
#if __STDC__ || defined(__cplusplus)
int
parse_event(char *word)
#else
parse_event(word)
  char           *word;
#endif

{
  if (strstr(word, "Start-Phase"))
    return (START_PHASE);
  if (strstr(word, "Start-Stop"))
    return (START_STOP);
  if (strstr(word, "Phase-Phase"))
    return (PHASE_PHASE);
  if (strstr(word, "Phase-Reset"))
    return (PHASE_RESET);
  if (strstr(word, "Phase-Unlock"))
    return (PHASE_UNLOCK);
  if (strstr(word, "Phase-Stop"))
    return (PHASE_STOP);
  if (strstr(word, "Reset-Phase"))
    return (RESET_PHASE);
  if (strstr(word, "Reset-Reset"))
    return (RESET_RESET);
  if (strstr(word, "Reset-Unlock"))
    return (RESET_UNLOCK);
  if (strstr(word, "Unlock-Phase"))
    return (UNLOCK_PHASE);
  if (strstr(word, "Unlock-Stop"))
    return (UNLOCK_STOP);
  if (strstr(word, "Stop-Start"))
    return (STOP_START);
  if (strstr(word, "Phase-Start"))
    return (PHASE_START);
  if (strstr(word, "Unlock-Start"))
    return (UNLOCK_START);
  if (strstr(word, "Reset-Start"))
    return (RESET_START);
  if (strstr(word, "Reset-Stop"))
    return (RESET_STOP);

  return (NO_EVENT);
}

int FreeRate(rate)
CLOCKRATE *rate;
{
	CLOCKRATE *rptr,*tmp;
	/* free up the structs */
	rptr= rate;
	for (rptr = rate; rptr!= NULL ; rptr = tmp) {
		tmp=rptr->next;
		free(rptr->comment);
		free(rptr);
	}
	return 1;
}

/* count the number of rate entries */
int
CountRate(rate)
  CLOCKRATE      *rate;
{
  CLOCKRATE      *cptr;
  int             count;
  count = 0;
  cptr = rate;
  while (cptr != NULL) {
    cptr = cptr->next;
    count++;
  }
  return count;
}

/* get last rate entry  */
CLOCKRATE
* LastRate(rate)
  CLOCKRATE      *rate;
{
  CLOCKRATE      *cptr;
  cptr = rate;
  while (cptr != NULL) {
    if (cptr->next == NULL)
      return cptr;
    cptr = cptr->next;
  }
  /* if we get to here we're in trouble */
  return NULL;
}
/* count the number of modified rate entries */
#if __STDC__ || defined(__cplusplus)
int
CountModRate(CLOCKRATE *rate)
#else
int
CountModRate(rate)
  CLOCKRATE      *rate;
#endif
{
  CLOCKRATE      *cptr;
  int             count;
  count = 0;
  cptr = rate;
  while (cptr != NULL) {
    if (cptr->modified == TRUE)
      count++;
    cptr = cptr->next;
  }
  return count;
}


/* 	ParseTimeString(time,epoch, string)

	takes a string of YY:JJJ:HH:MM[:SS[.SSS]] and parses it into
	a time_struct and an long epoch both of which are passed as pointers.
	Returns TRUE upon parsing a successful time
	or FALSE otherwise.
	A successful time could have the seconds and/or milliseconds 
	left off (i.e., YY:JJJ:HH:MM:SS and YY:JJJ:HH:MM are both
	valid times for the ParseTimeString() function .
*/
int
ParseTimeString(time,epoch, string)
struct time_type *time;
long *epoch;
char *string;
{

int i;
struct epoch_type e;
	i = sscanf(string, "%2hd:%3hd:%2hd:%2hd:%2hd.%3hd", &(time->year), 
		&(time->day), &(time->hour), &(time->min), &(time->sec), &(time->msec) );
	if (i!=6 && i!=5 && i!=4) return FALSE;
	if (i<=5) time->msec = 0;
	if (i==4) time->sec = 0;
	if(time->year>99 || time->year<0)
		return FALSE;
	time->year += 1900;
	if(time->hour>23 || time->hour<0)
		return FALSE;
	if(time->min>59 || time->min<0)
		return FALSE;
	if(time->sec>59 || time->sec<0)
		return FALSE;
	if(time->msec>999 || time->msec<0)
		return FALSE;
	gmt2epoch(&e, time, 0);
	*epoch = e.sec;
	return TRUE;
}
/* 	ParseLine(line, white)

	returns an array of pointers to strings in line
	separated by whitespace in the string white.
	presently the array is limited to ten items.
	If the first char is a #, then NULL is returned.
 */
#define MAX_ITEMS 10
char **ParseLine(line, white) 
char *line, *white;
{
char **list, *item, *c_ptr;
int i;
	
	if( line[0] == '#')
	    return NULL;	
	/* lose the newline , NEED TO MAKE THIS CLEANER...*/
	if((c_ptr =strchr(line, '\n')) !=NULL)
		*c_ptr = '\0';
	list = (char **) calloc (MAX_ITEMS, sizeof(char*));
	item = strtok(line, white);
	if (item==NULL)
	    return NULL;	
	list[0] = item;
	i=1;
	while( (item=strtok(NULL, white)) !=NULL) {
		list[i++] = item;
		if(i>MAX_ITEMS)
			return list;
	}
	return list;
}

/* add's the delay value, in milliseconds to the time and epoch structures */

AddStartDelay(time, epoch, delay) 
struct time_type *time;
long *epoch;
int delay;
{
struct epoch_type e;
double sec;
	sec = (time->msec/1000. + delay/1000.);
	e.sec = *epoch + (int)(sec);  /* secs */
	*epoch = e.sec;
	e.usec = 1000 * (((int)(sec*1000))%1000); /* micro secs */
	epoch2gmt(time, &e, 0);
}

/* returns the total shift in milliseconds */
int
get_total_shift(entry, epoch, correction)
double *correction;
CLOCKRATE *entry;
double epoch;
{
int trshift;

*correction = (epoch - entry->epoch) * entry->slope;
trshift = (int) rint((double) ((entry->offset+*correction)*1000.));

return trshift;
}

