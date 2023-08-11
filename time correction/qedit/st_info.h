/************************************************************************/
/* Input and output channel info structure.				*/
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

/*	$Id: st_info.h,v 1.7 2003/07/12 00:26:14 doug Exp $ 	*/

/************************************************************************/
/*  Channel information structure.  One allocated for each file.	*/
/*  This is used to store channel information in a manner that is	*/
/*  independent of both the input and output format.			*/
/************************************************************************/

#include <sys/param.h>
#include "drm_seismo.h"

#define	ST_CHANNEL_LEN	5
#define	ST_STATION_LEN	8

typedef struct _iob {
    int		    fd;		/* file descriptor.			*/
    int		    bufsize;	/* size of buffer.			*/
    int		    avail;	/* index of next input byte.		*/
    int		    free;	/* index of next free byte.		*/
    char	    buf[BUFSIZE];	/* actual data buffer.		*/
} IOB;

typedef struct _st_info {
    char	    station[8]; /* name of station.			*/
    char	    channel[4];	/* name of channel.			*/
    char	    network[4];	/* name of network.			*/
    char	    filename[MAXPATHLEN];   /* filename of input file.	*/
    INT_TIME	    begtime;	/* beginning time of data stream.	*/
    INT_TIME	    endtime;	/* end time of data stream.		*/
    int		    hdr_type;	/* type of input data stream.		*/
    int		    blksize;	/* input blocksize.			*/
    int		    rate;	/* data rate of stream.			*/
    int		    rate_mult;	/* data rate_mult of stream.		*/
    int		    num_blk;	/* # of data block read for this comp.	*/
    int		    num_samples;/* # of samples, pointed to by data.	*/
    int		    num_blocks;	/* # of blocks.				*/
    FILE	    *fp;	/* file pointer for data file.		*/
    DATA_HDR	    *cur_hdr;	/* ptr to current header structure.	*/
    IOB		    *iob;	/* i/o buffer for buffered i/o.		*/
    STORE_FILE_HEAD *sfh;	/* store_file_head ptr for drm file.	*/
    char	    *vfh;	/* volume file hdr for telemetry files.	*/
    BLKINFO	    *first;	/* ptr to first blkinfo structure.	*/
    BLKINFO	    *last;	/* ptr to last blkinfo structure.	*/
    BLKINFO	    *cur;	/* ptr to current blkinfo structure.	*/
    BLKINFO	    *prevpos;	/* ptr to previous blkinfo structure.	*/
    struct _st_info *next;	/* ptr to next stream structure in list.*/
    struct _st_info *prev;	/* ptr to prev stream structure in list.*/
} ST_INFO;
