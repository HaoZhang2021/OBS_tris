/************************************************************************/
/*  Routines to get information about the input stream.			*/
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
static char sccsid[] = "$Id: get_stream_info.c,v 1.6 2000/10/26 13:39:37 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/*  get_stream_info:							*/
/*	Get information about the input data stream.			*/
/************************************************************************/
int get_stream_info 
   (ST_INFO	*st_p)		/* ptr to current stream header.	*/
{
    STORE_FILE_HEAD *sfh;
    STORE_DATA	    *sd;
    QDA_HDR	    *qh;
    SDR_HDR	    *sh;
    INT_TIME	    *pt;
    DATA_HDR	    *hdr;
    char	    str[SDR_SEQ_LEN+1];
    int		    seq_no, next_seq;
    int		    i;
    int		    nb;
    int		    skip;
    char	    *p, *q;

    st_p->blksize = 0;
    if (fill_buffer (st_p, MAX_BLKSIZE) == 0) return (EOF);

    /*	Determine the type of the file, and fill in header information.	*/
    /*	Get: 
    /*	    header_type, station, stream, sample rate, and blksize.	*/

    st_p->hdr_type = get_header_type (st_p->iob->buf, st_p->iob->free);

    switch (st_p->hdr_type) {
    case DRM_HDR_TYPE:
	sfh = (STORE_FILE_HEAD *)(st_p->iob->buf);
	if ( (st_p->sfh = (STORE_FILE_HEAD *)malloc(sizeof(STORE_FILE_HEAD))) == NULL) {
	    fprintf (info, "unable to malloc sfh structure.\n");
	    exit(1);
	}
	memcpy ((void *)st_p->sfh, (void *)sfh, sizeof(STORE_FILE_HEAD));
	strcpy(st_p->station, sfh->stream_config.station_name);
	strcpy(st_p->channel, sfh->stream_config.seedchannel);
	strcpy(st_p->network, "  ");
	trim (st_p->station);
	trim (st_p->channel);
	trim (st_p->network);
	st_p->blksize =  sfh->store_pack_size;
	if (st_p->blksize != sizeof (STORE_DATA)) {
	    fprintf (info, "blksize says %d, should be %d\n", st_p->blksize,
		     sizeof (STORE_DATA));
	}
	if (sfh->store_file_type != STYPE_CONT) {
	    fprintf (info, "filetype = %d, should be %d\n", 
		     sfh->store_file_type, STYPE_CONT);
	    exit(1);
	}
	st_p->begtime = decode_time_drm (sfh->store_begin, SEED_BIG_ENDIAN);
	st_p->endtime = decode_time_drm (sfh->store_end, SEED_BIG_ENDIAN);

	/* Read the first block to get the rest of the info.		*/
	/* Rate is ONLY available in the block header.			*/
	/* The first block begins at offset				*/
	/*	store_offset*store_pack_size				*/
	/* in the file.							*/
	st_p->iob->avail = sizeof(STORE_FILE_HEAD);
	skip = (sfh->store_offset * sfh->store_pack_size)- sizeof(STORE_FILE_HEAD);
	if (fill_buffer (st_p, skip) < skip) return(-1);
	st_p->iob->avail = skip;
	if (fill_buffer (st_p, st_p->blksize) < st_p->blksize) return(-1);
	sd = (STORE_DATA *)(st_p->iob->buf);
	st_p->rate = sd->rate;
	st_p->rate_mult = 1;
	break;

    case QDA_HDR_TYPE:
	qh = (QDA_HDR *)(st_p->iob->buf);
	hdr = decode_hdr_qda (qh, st_p->iob->free);
	strcpy (st_p->station, hdr->station_id);
	strcpy (st_p->channel, hdr->channel_id);
	strcpy (st_p->network, hdr->network_id);
	st_p->rate = hdr->sample_rate;
	st_p->rate_mult = 1;
	seq_no = hdr->seq_no;
	st_p->blksize = hdr->blksize;
	if (st_p->blksize == 0) {
	    fprintf (stderr, "invalid blocksize for %s\n", st_p->filename);
	    exit(1);
	}
	free_data_hdr (hdr);
	break;

    case SDR_VOL_HDR_TYPE:
	sh = (SDR_HDR *)(st_p->iob->buf);
	hdr = decode_hdr_sdr(sh,st_p->iob->free);
	st_p->blksize = hdr->blksize;
	free_data_hdr (hdr);
	if ( (st_p->vfh = (char *)malloc(st_p->blksize+1)) == NULL) {
	    fprintf (info, "unable to malloc vfh structure.\n");
	    exit(1);
	}
	memcpy ((void *)st_p->vfh, (void *)sh, st_p->blksize);
	*((st_p->vfh) + st_p->blksize) = '\0';

	p = (char *)(st_p->iob->buf)+31;
	q = strchr(p,'~');
	*q = '\0';
	if ((pt = parse_date(p)) != NULL) st_p->begtime = *pt;
	else fprintf (info, "Unable to get start time from hdr\n");
	p = q+1;
	q = strchr(p,'~');
	*q = '\0';
	if ((pt = parse_date(p)) != NULL) st_p->endtime = *pt;
	else fprintf (info, "Unable to get end time from hdr\n");

	/* Read the first data block to get the rest of the info.	*/
	st_p->iob->avail = st_p->blksize;
	if (fill_buffer (st_p, st_p->blksize) < st_p->blksize) return(-1);
	/* Fall through and pick up info from first SEED data record. */
	st_p->hdr_type = SDR_HDR_TYPE;

    case SDR_HDR_TYPE:
	sh = (SDR_HDR *)(st_p->iob->buf);
	hdr = decode_hdr_sdr(sh,st_p->iob->free);
	strcpy (st_p->station, hdr->station_id);
	strcpy (st_p->channel, hdr->channel_id);
	strcpy (st_p->network, hdr->network_id);
	st_p->rate = hdr->sample_rate;
	st_p->rate_mult = hdr->sample_rate_mult;
	st_p->blksize = hdr->blksize;
	free_data_hdr(hdr);
	if (st_p->blksize > MAX_BLKSIZE) {
	    fprintf(stderr, "Fatal Error, Block size %d is greater than MAX_BLKSIZE %d\n",
		    st_p->blksize, MAX_BLKSIZE);
	    exit(1);
	}
	if (st_p->blksize == 0) {
	    fprintf (stderr, "invalid blocksize for %s\n", st_p->filename);
	    exit(1);
	}
	break;

    default:
	fprintf (stderr, "unrecognizable input file %s\n", st_p->filename);
	exit(1);
	break;
    }
    return(0);
}

/************************************************************************/
/*  get_header_type:							*/
/*	Determine the type of input stream.				*/
/************************************************************************/
int get_header_type 
   (char    *p,			/* ptr to buffer containing header.	*/
    int	    nr)			/* max number of bytes for header.	*/
{
    if (is_drm_header((STORE_FILE_HEAD *)p,nr)) return DRM_HDR_TYPE;
    else if (is_qda_header((QDA_HDR *)p,nr)) return QDA_HDR_TYPE;
    else if (is_sdr_header((SDR_HDR *)p,nr)) return SDR_HDR_TYPE;
    else if (is_sdr_vol_header((SDR_HDR *)p,nr)) return (SDR_VOL_HDR_TYPE);
    else return (-1);
}
