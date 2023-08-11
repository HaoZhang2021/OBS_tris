/************************************************************************/
/*  Routines to position input stream.					*/
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
static char sccsid[] = "$Id: position_stream.c,v 1.3 2000/10/26 13:39:39 doug Exp $ ";
#endif

#include <stdio.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/*  position_stream:							*/
/*	Position the input stream to the first block that does not	*/
/*	preceed the specified time.					*/
/************************************************************************/
int position_stream 
   (ST_INFO	*st_p,		/* ptr to current input stream.		*/
    INT_TIME	*stime,		/* ptr to desired start time.		*/
    INT_TIME	*etime)		/* ptr to desired end time.		*/
{
    int		nr;

    while (1) {
	/* Get a new block if necessary.    */
	if (st_p->cur_hdr == NULL) {
	    /* Ensure that we have a block of data cached.		*/
	    if ( (nr = fill_buffer (st_p, st_p->blksize)) < st_p->blksize ) {
		if (nr != 0) fprintf (info, "partial block of %d bytes at end of %s\n",
				      nr, st_p->filename);
		return(EOF);
	    }
	    get_block_info (st_p);	    /* examine the block.	*/
	}

	/* if end_time <= block start_time, return EOF.	*/
	if ((etime != NULL) && (tdiff(*etime,st_p->cur_hdr->begtime) <= 0))
	    return (EOF);

	/* If no time is specified, just return with the current block.	*/
	if (stime == NULL) return(0);

	/* If start_time <= block endtime, we are done.	*/
	if (tdiff(*stime,st_p->cur_hdr->endtime) <= 0 ) break;

	/* Otherwise, get the next block.   */
	free_data_hdr (st_p->cur_hdr);
	st_p->cur_hdr = NULL;
	st_p->iob->avail = st_p->iob->free;
    }
    return(0);
}
