/************************************************************************/
/*  Routines to get block information from active file.			*/
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
static char sccsid[] = "$Id: get_block_info.c,v 1.6 2000/10/26 13:39:37 doug Exp $ ";
#endif

#include <stdio.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/*  get_block_info:							*/
/*	Get information about the next block in stream.			*/
/************************************************************************/
void get_block_info
   (ST_INFO	*st_p)		/* ptr to current stream header.	*/
{
    int	    *p;
    int	    steim_comp;
    BS		    *bs;		/* ptr to blockette structure.	*/
    BLOCKETTE_HDR   *bh;		/* ptr to blockette hdr.	*/
    static int warned_frame_count = 0;
    double   sps;

    if (my_wordorder < 0)
	get_my_wordorder();
    switch (st_p->hdr_type) {
    case DRM_HDR_TYPE:
	st_p->cur_hdr = decode_hdr_drm ((STORE_DATA *)st_p->iob->buf,
					st_p->iob->free);
	if (st_p->cur_hdr == NULL) {
	    fprintf (stderr, "unable to get header for block %d from stream %s\n", 
		     st_p->num_blocks+1, st_p->filename);
	    exit(1);
	}
	strcpy (st_p->cur_hdr->station_id, st_p->station);
	strcpy (st_p->cur_hdr->channel_id, st_p->channel);
	strcpy (st_p->cur_hdr->location_id, "");
	strcpy (st_p->cur_hdr->network_id, "");
	break;
    case QDA_HDR_TYPE:
	st_p->cur_hdr = decode_hdr_qda ((QDA_HDR *)st_p->iob->buf,
					st_p->iob->free);
	break;
    case SDR_HDR_TYPE:
	st_p->cur_hdr = decode_hdr_sdr ((SDR_HDR *)st_p->iob->buf,
					st_p->iob->free);
	break;
    default:
	fprintf (stderr, "unknown header type for block %d from stream %s\n", 
		 st_p->num_blocks+1, st_p->filename);
	exit(1);
	break;
    }
    if (st_p->cur_hdr == NULL) {
	fprintf (stderr, "unable to get header from block %d from stream %s\n", 
		 st_p->num_blocks+1, st_p->filename);
	exit(1);
    }
    st_p->blksize = st_p->cur_hdr->blksize;
    sps = sps_rate (st_p->cur_hdr->sample_rate, st_p->cur_hdr->sample_rate_mult);

    /* Assume data steim compressed unless explicitly told otherwise.	*/
    steim_comp = ((bs = find_blockette(st_p->cur_hdr,1000)) &&
	(bh = (BLOCKETTE_HDR *)(bs->pb)) &&
	! (IS_STEIM_COMP(((BLOCKETTE_1000 *)bh)->format))) ? 0 : 1;

    /*:: Bug fix for missing frame count in comserv shear data. */
    if (steim_comp && (bs = find_blockette(st_p->cur_hdr,1001)) &&
	(bh = (BLOCKETTE_HDR *)(bs->pb)) &&
	((BLOCKETTE_1001 *)bh)->frame_count == 0 &&
	st_p->cur_hdr->num_samples > 0 && 
	sps != 0.0) {
	((BLOCKETTE_1001 *)bh)->frame_count =
	    (st_p->blksize - st_p->cur_hdr->first_data) / sizeof(FRAME);
	if (! warned_frame_count) fprintf (stderr, "Warning - computing missing frame count for blockette 1001.\n");
	warned_frame_count = 1;
    }

    /* Fill in the number of data frames.   */
    if (steim_comp && (bs = find_blockette(st_p->cur_hdr,1001)) &&
	(bh = (BLOCKETTE_HDR *)(bs->pb))) 
	st_p->cur_hdr->num_data_frames = ((BLOCKETTE_1001 *)bh)->frame_count;
    else st_p->cur_hdr->num_data_frames =
	(sps == 0.0 || ! steim_comp) ? 0 :
	(st_p->blksize - st_p->cur_hdr->first_data) / sizeof(FRAME);

    /*	Fill in x0 and xn if we know (or assume) the data is in		*/
    /*	Steim compressed format.					*/
    st_p->cur_hdr->x0 = 0;
    st_p->cur_hdr->xn = 0;
    if (steim_comp && st_p->cur_hdr->num_samples > 0 && sps != 0.0) {
	bs = find_blockette(st_p->cur_hdr,1000);
	bh = (bs) ? (BLOCKETTE_HDR *)(bs->pb) : (BLOCKETTE_HDR *)NULL;
	if ((bs == NULL) || (bs && (((BLOCKETTE_1000 *)bh)->format == STEIM1 ||
				    ((BLOCKETTE_1000 *)bh)->format == STEIM2))) {
	    p = (int *)(st_p->iob->buf + st_p->cur_hdr->first_data + 4);
	    st_p->cur_hdr->x0 = *(p++);
	    st_p->cur_hdr->xn = *(p++);
	    if (my_wordorder != st_p->cur_hdr->data_wordorder) {
		swab4 (&st_p->cur_hdr->x0);
		swab4 (&st_p->cur_hdr->xn);
	    }
	}
    }
    ++(st_p->num_blocks);
}
