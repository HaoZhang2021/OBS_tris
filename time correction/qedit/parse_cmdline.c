/************************************************************************/
/*  Command line parsing routines.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef lint
static char sccsid[] = "$Id: parse_cmdline.c,v 1.4 2000/10/26 13:39:39 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

#define	argc	(*pargc)
#define	argv	(*pargv)

/************************************************************************/
/*  parse_cmdline:							*/
/*	Parse command line.						*/
/************************************************************************/
int parse_cmdline 
   (int		*pargc,		/* ptr to argc.				*/
    char	***pargv)	/* ptr to argv.				*/
{
    char    *span;
    int	    span_flag = 0;
    char    *p;
    INT_TIME *tp;

    /* Variables needed for getopt. */
    extern char	*optarg;
    extern int	optind, opterr;
    int		c;

    int i;
    int	nfiles;

    cmdname = tail(*argv);

    /*	Parse command line options.					*/
    while ( (c = getopt(argc,argv,"hnTcd:o:i:f:t:s:D:")) != -1)
	switch (c) {
	case '?':
	case 'h':   print_syntax(cmdname,syntax,info); exit(0); break;
	case 'd':   debug_option = atoi(optarg); break;
	case 'f':
	    if ((tp = parse_date(optarg)) == NULL) {
		fprintf (info, "invalid date: %s\n", optarg);
		exit(1);
	    }
	    start_time = *tp;
	    ++start_flag;
	    break;
	case 't':
	    if ((tp = parse_date(optarg)) == NULL) {
		fprintf (info, "invalid date: %s\n", optarg);
		exit(1);
	    }
	    end_time = *tp;
	    ++end_flag;
	    break;
	case 's':
	    span = optarg; ++span_flag; break;
	case 'D':
	    /* Date display format.   */
	    if (strcmp (optarg, "j") == 0) date_fmt = JULIAN_FMT;
	    else if (strcmp (optarg, "j,") == 0) date_fmt = JULIAN_FMT_1;
	    else if (strcmp (optarg, "m") == 0) date_fmt = MONTH_FMT;
	    else if (strcmp (optarg, "m,") == 0) date_fmt = MONTH_FMT_1;
	    else {
		fprintf (info, "invalid format for date display: %s\n", optarg);
		exit(1);
	    }
	    break;
	}

    if (end_flag && span_flag) {
	fprintf (stderr, "span and endtime mutually exclusive\n");
	exit(1);
    }
    if (span_flag && !start_flag) {
	fprintf (stderr, "span not valid without start time\n");
	exit(1);
    }
    if (span_flag) {
	if (! valid_span (span)) {
	    fprintf (stderr, "invalid span specification: %s\n", span);
	    exit(1);
	}
	end_time = end_of_span (start_time, span);
	++end_flag;
    }

    /*	The output stream will get opened only when we write the file.	*/

    /*	Skip over all options and their arguments.			*/
    argv = &(argv[optind]);
    argc -= optind;

    /* The remaining arguments are input files and optional output file.*/
    /* Open input file and allocate the appropriate data structures.	*/

    nfiles = argc;
    switch (nfiles) {
      case 0:	break;
      case 1:
      case 2:
	open_stream (&st_head_in, *argv++, "r");
	--argc;
	outfile = (argc > 0) ? *argv++ : NULL;
	break;
      default:
	fprintf (info, "too many filenames\n");
	exit(1);
    }
    return(0);
}
