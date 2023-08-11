/************************************************************************/
/*  Routine to store record in memory.					*/
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
static char sccsid[] = "$Id: store_block.c,v 1.4 2000/10/26 13:39:41 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/* store_block:								*/
/*	store most recently read block for input stream.		*/
/************************************************************************/
void store_block 
   (ST_INFO	*st_p)		/* ptr to input stream struct.		*/
{
    BLKINFO	*bp;
    char	*p;
    int		bl;

    /*	Allocate space for linked list of header and data points.	*/
    if ((bp = (BLKINFO *)malloc(sizeof(BLKINFO))) == NULL) {
	fprintf (info, "unable to malloc st_info structure.\n");
	exit(1);
    }
    /*	Allocate space for data block.					*/
    bp->data = NULL;
    bp->datalen = 0;
    if (st_p->cur_hdr->first_data > 0) 
	bp->datalen = st_p->blksize - st_p->cur_hdr->first_data;
    if ((bp->data = (char *)malloc(bp->datalen)) == NULL) {
	fprintf (info, "unable to malloc data structure.\n");
	exit(1);
    }

    /*	Fill in block info and copy data.				*/
    bp->hdr = st_p->cur_hdr;
    if (bp->data != NULL) 
	memcpy (bp->data, st_p->iob->buf+st_p->cur_hdr->first_data, bp->datalen);

    /*	Chain block info into doubly linked list.			*/
    if (st_p->last == NULL) {
	/* This is our first info for this data stream.			*/
	bp->next = NULL;
	bp->prev = NULL;
	st_p->first = bp;
	st_p->last = bp;
	bp->blk_no = 1;
    }
    else {
	bp->prev = st_p->last;
	bp->next = NULL;
	st_p->last->next = bp;
	st_p->last = bp;
	bp->blk_no = bp->prev->blk_no + 1;
    }
    st_p->cur = bp;

    /*	Remove header from direct st_p pointer, and step I/O buffers	*/
    /*	over the copied data.						*/
    st_p->cur_hdr = NULL;
    st_p->iob->avail = st_p->blksize;
}
