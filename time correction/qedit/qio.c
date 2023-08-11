/************************************************************************/
/*  I/O routines.							*/
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
static char sccsid[] = "$Id: qio.c,v 1.3 2000/10/26 13:39:40 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

#define	info    stderr

/************************************************************************/
/*  open_stream:							*/
/*	Open the specified stream, and allocate all structures		*/
/*	required for its support.					*/
/************************************************************************/
ST_INFO *open_stream
   (ST_INFO	*phead,		/* ptr to stream struct linked list.	*/
    char	*file,		/* filename to open.			*/
    char	*mode)		/* mode for open.			*/
{
    ST_INFO *p;
    int	    fmode;

    fmode = (mode[0] == 'r') ? O_RDONLY : 
	    (mode[0] == 'a') ? (O_WRONLY | O_CREAT | O_APPEND) :
	    (mode[0] == 'w') ? (O_WRONLY | O_CREAT | O_TRUNC) :
	    (O_WRONLY | O_CREAT | O_TRUNC);
    if ( (p = (ST_INFO *)malloc(sizeof(ST_INFO))) == NULL) {
	fprintf (info, "unable to malloc st_info structure.\n");
	exit(1);
    }
    memset ((void *)p, 0, sizeof(ST_INFO));
    if ( (p->iob = (IOB *)malloc(sizeof(IOB))) == NULL ) {
	fprintf (info, "unable to malloc iob structure.\n");
	exit(1);
    }
    memset ((void *)p->iob, 0, sizeof(IOB));
    if (file == NULL) {
	if (strcmp(mode,"r")==0) {
	    strcpy (p->filename, "<stdin>");
	    p->iob->fd = fileno(stdin);
	}
	else {
	    strcpy (p->filename, "<stdout>");
	    p->iob->fd = fileno(stdout);
	}
    }
    else {
	strcpy (p->filename, file);
	if ( (p->iob->fd = open (p->filename, fmode, 0666)) < 0 ) {
	    fprintf (info, "unable to open file %s\n", p->filename);
	    exit(1);
	}
    }
    /* Link this structure into the linked list.  Insert the new	*/
    /* structure at the head of the list, between the header and the	*/
    /* previous first element.						*/
    p->next = phead->next;	/* point forward to old first struct.	*/
    p->prev = phead;		/* point back to head.			*/
    phead->next = p;		/* point head to me.			*/
    if (p->next != NULL) {
	p->next->prev = p;	/* point old first struct back to me.	*/
    }

    return (p);
}

/************************************************************************/
/*  close_stream:							*/
/*	Close the specified stream, and deallocate all structures	*/
/*	required for its support.					*/
/*  returns:								*/
/*	Addr of prev stream structure from linked list.			*/
/************************************************************************/
ST_INFO *close_stream
   (ST_INFO	*st_p)		/* ptr to stream struct to close.	*/
{
    ST_INFO *prev;
    /*	Close the stream.   */
    close (st_p->iob->fd);

    /*  Unchain this ST_INFO from the linked list.			*/
    /*  We ALWAYS have a prev structure, since there is a dummy head	*/
    /*	for the list, but not always a next struct.			*/
    st_p->prev->next = st_p->next;    /* unlink fwd pointer.		*/
    if (st_p->next != NULL) st_p->next->prev = st_p->prev;

    /* Save pointer to this stream's prev so we can return it as the	*/
    /* function value.							*/
    prev = st_p->prev;

    /* Now deallocate all info that was allocated for this stream.	*/
    free (st_p->iob);
    if (st_p->cur_hdr) free_data_hdr (st_p->cur_hdr);
    while (st_p->first != NULL) {
	if (st_p->first->data != NULL) free (st_p->first->data);
	if (st_p->first->hdr != NULL) free_data_hdr (st_p->first->hdr);
	st_p->cur = st_p->first->next;
	free (st_p->first);
	st_p->first = st_p->cur;
    }
    free (st_p);
    return (prev);
}

/************************************************************************/
/*  fill_buffer:							*/
/*	Ensure that the input buffer has the specified amount of input	*/
/*	in its input buffer.						*/
/************************************************************************/
int fill_buffer
   (ST_INFO	*st_p,		/* ptr to input stream struct.		*/
    int		n)		/* number of bytes desired in IOB.	*/
{
    int		*pfree = &(st_p->iob->free);
    int		*pavail = &(st_p->iob->avail);
    int		nbytes_in_buf = *pfree - *pavail;
    int		nread;

    /*	Want to ensure that at least N bytes are in the buffer.		*/
    /*	In addition, shift data in the buffer so that it starts		*/
    /*	at the beginning of the buffer.					*/
    /*	This way we ensure that there is sufficient room.		*/

    /*	Shift data in buffer (if necessary).				*/
    if (nbytes_in_buf < 0) nbytes_in_buf = 0;
    if (nbytes_in_buf > 0 && *pavail > 0)
	/* WARNING - OVERLAPPING COPY.  memcpy cannot be used.		*/
	bcopy (st_p->iob->buf+*pavail, st_p->iob->buf, nbytes_in_buf);
    *pavail = 0;
    *pfree = nbytes_in_buf;
    
    /*	Add data to the buffer (if necessary).				*/
    n -= nbytes_in_buf;
    if (n > 0) {
	nread = xread (st_p->iob->fd, st_p->iob->buf+nbytes_in_buf, n);
	if (nread > 0) *pfree += nread;
    }
    return (*pfree);
}

/************************************************************************/
/*  write_stream:							*/
/*	Write output block to output stream, and update summary		*/
/*	counter info.							*/
/************************************************************************/
void write_stream
   (ST_INFO	*out)		/* ptr to output stream struct.		*/
{
    int		nw;
    int		blksize = out->blksize;
    if ( (nw = xwrite (out->iob->fd, out->iob->buf, blksize)) != blksize ) {
	fprintf (stderr, "error writing output unit %d for file %s\n", 
		 out->iob->fd, out->filename);
	fprintf (stderr, "tried to write %d bytes, wrote %d\n", blksize, nw);
	exit(1);
    }
    /* Update information in the main stream structure.			*/
    ++(out->num_blk);
    out->num_samples += out->cur_hdr->num_samples;
}
