/************************************************************************/
/*  Routines for writing records to file.				*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2003 The Regents of the University of California.
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
static char sccsid[] = "$Id: write_file.c,v 1.8 2003/07/12 00:26:14 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/*  do_write_file:							*/
/*	Write output file.  Currently supports only DRM and SDR files.	*/
/************************************************************************/
void do_write_file
   (ST_INFO	*st_p,		/* ptr to input stream struct.		*/
    CMD		*cmd,		/* ptr to command structure.		*/
    char	*filename)	/* output filename.			*/
{
    ST_INFO	*out;
    BLKINFO	*bp;
    char	answer[80];
    char	*newfilename = (cmd->arg[0].argt == STR_TYPE) ? cmd->arg[0].argv.str : NULL;

    if (debug(DEBUG_ANY)) fprintf (info, "autodisplay command\n");
    if (newfilename) filename = newfilename;
    if (filename == NULL) {
	fprintf (info, "OK to overwrite input file %s ? ", st_p->filename);
	scanf ("%s", answer);
	if (strncasecmp(answer,"Y",1)!= 0) {
	    fprintf (info, "Will not overwrite input file: filename\n", st_p->filename);
	    return;
	}
	else filename = st_p->filename;
    }

    if ((out = open_stream(&st_head_out, filename, "w")) == NULL) {
	fprintf (info, "Unable to open output file %s\n", filename);
	return;
    }
    fprintf (info, "Writing file: %s\n", filename);

    /*	For the moment, don't even bother with any BLKINFO, since all	*/
    /*	we need to do is write to our iob.				*/
    /*	We need a cur_hdr for summary info, however.			*/
    bp = st_p->cur;
    out->cur_hdr = new_data_hdr();

    switch (st_p->hdr_type) {
    case DRM_HDR_TYPE:
	write_file_drm (st_p, out); break;
    case QDA_HDR_TYPE:
	/*  Currently let this default to writing SDR output files.	*/
    case SDR_HDR_TYPE:
	write_file_sdr (st_p, out); break;
    default:
	fprintf (stderr, "unable to write file -- Unknown header type\n");
    }
    close_stream(out);
    if (cmd->op == FILEQUIT) exit(0);
    st_p->cur = bp;
    return;
}

/************************************************************************/
/*  write_file_sdr:							*/
/*	Write file in SDR format.					*/
/************************************************************************/
void write_file_sdr 
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out)		/* ptr to output stream structure.	*/
{
    int		o_offset, datalen;

    /*	For the moment, write out SEED data records with the same	*/
    /*	blocksize as the input file.					*/
    /*  Write out new volume header if one existed before.		*/
    write_vol(st_p, out);
    st_p->cur = st_p->first;
    while (st_p->cur != NULL) {
	memset ((void *)out->iob->buf, 0, st_p->blksize);
	init_sdr_hdr ((SDR_HDR *)out->iob->buf, st_p->cur->hdr, NULL);
	o_offset = st_p->cur->hdr->first_data;
	datalen = st_p->cur->datalen;
	out->blksize = st_p->blksize;
	if (datalen > 0) {
	    memcpy (out->iob->buf+o_offset, st_p->cur->data, datalen);
	}
	update_sdr_hdr ((SDR_HDR *)out->iob->buf, st_p->cur->hdr);
	write_stream (out);
	st_p->cur = st_p->cur->next;
    }
}

/************************************************************************/
/*  write_file_drm:							*/
/*	Write file in DRM format.					*/
/************************************************************************/
void write_file_drm
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out)		/* ptr to output stream structure.	*/
{
    char	*chan = NULL;
    char	*stat = NULL;
    char	*loc = NULL;
    int		n_blocks = 0;
    STORE_DATA	*sdh;
    int		o_offset, datalen;

    /*  Output the file config hearder.  Ensure all info is correct.	*/
    /*	Certain info such as station, channel, and location MUST be	*/
    /*	consistent throughout the file, since they are represented only	*/
    /*	once in the file header.					*/
    if ((st_p->cur = st_p->first) && st_p->cur->hdr) {
	stat = st_p->cur->hdr->station_id;
	chan = st_p->cur->hdr->channel_id;
	loc = st_p->cur->hdr->location_id;
    }
    if (st_p->cur == NULL) {
	fprintf (info, "no datablocks for file, unable to write\n");
	return;
    }
    while (st_p->cur != NULL) {
	if (strcmp(stat, st_p->cur->hdr->station_id)) {
	    fprintf (info, "station mismatch at block %d, seq %d\n", 
		     st_p->cur->blk_no, st_p->cur->hdr->seq_no);
	    return;
	}
	if (strcmp(chan, st_p->cur->hdr->channel_id)) {
	    fprintf (info, "channel mismatch at block %d, seq %d\n", 
		     st_p->cur->blk_no, st_p->cur->hdr->seq_no);
	    return;
	}
	if (strcmp(loc, st_p->cur->hdr->location_id)) {
	    fprintf (info, "location mismatch at block %d, seq %d\n", 
		     st_p->cur->blk_no, st_p->cur->hdr->seq_no);
	    return;
	}
	++n_blocks;
	st_p->cur = st_p->cur->next;
    }
    /*	Update file header information and write file header.		*/
    /*	The file header uses an integral number of packets.		*/
    strncpy (st_p->sfh->stream_config.seedchannel,chan,3);
    strncpy (st_p->sfh->stream_config.station_name,stat,4);
    st_p->sfh->store_count = n_blocks;
    st_p->sfh->store_begin = encode_time_drm(st_p->first->hdr->begtime, 
					     SEED_BIG_ENDIAN);
    st_p->sfh->store_end = encode_time_drm(st_p->last->hdr->endtime,
					   SEED_BIG_ENDIAN);
    out->blksize = st_p->sfh->store_pack_size * st_p->sfh->store_offset;
    memset(out->iob->buf, 0, out->blksize);
    memcpy (out->iob->buf, st_p->sfh, sizeof(STORE_FILE_HEAD));
    write_stream (out);

    /*	Convert each block back to a drm block, and write block.	*/
    out->blksize = st_p->sfh->store_pack_size;
    st_p->cur = st_p->first;
    while (st_p->cur != NULL) {
	memset ((void *)out->iob->buf, 0, st_p->blksize);
	sdh = encode_hdr_drm(st_p->cur->hdr);
	o_offset = (char *)&(sdh->da_d[0][0]) - (char *)sdh;
	/*  Copy only the header portion of the STORE_DATA.		*/
	memcpy (out->iob->buf, sdh, o_offset);
	datalen = st_p->cur->datalen;
	memcpy (out->iob->buf+o_offset, st_p->cur->data, datalen);
	out->blksize = o_offset + datalen;
	write_stream (out);
	st_p->cur = st_p->cur->next;
    }
}

#define VOLLEN_008		(8 + 3+4+4+2+5+2+3+23+23+1+1+2)
/************************************************************************/
/*  write_telemetry_vol:						*/
/*	Write a SEED telemetry volume header and blockette.		*/
/************************************************************************/
void write_vol
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out)		/* ptr to output stream structure.	*/
{
    char *vol;
    EXT_TIME begtime;
    EXT_TIME endtime;
    int blksize, lrl;
    int status = 1;

    if (st_p->vfh == NULL) return;
    blksize = st_p->blksize;
    lrl = log2((double)blksize);
    if ((vol = (char *)malloc(st_p->blksize+1))==NULL) {
	fprintf (info, "Unable to malloc new volume header\n");
	exit(1);
    }
    memset (vol,' ',st_p->blksize);
    begtime = int_to_ext(st_p->first->hdr->begtime);
    endtime = int_to_ext(st_p->last->hdr->endtime);
    /* Pick up the info from the first block in case these items have	*/
    /* been edited.							*/
    sprintf(vol,"%06d%c%c%03d%04d%4.1f%02d%-5.5s%-2.2s%-3.3s%04d,%03d,%02d:%02d:%02d.%04d~%04d,%03d,%02d:%02d:%02d.%04d~~~%-2.2s",
	    1, 'V', ' ', 8, VOLLEN_008-8, 2.3, lrl,
	    st_p->first->hdr->station_id, st_p->first->hdr->location_id, 
	    st_p->first->hdr->channel_id, 
	    begtime.year, begtime.doy, begtime.hour,
	    begtime.minute, begtime.second, begtime.usec/USECS_PER_TICK,
	    endtime.year, endtime.doy, endtime.hour,
	    endtime.minute, endtime.second, endtime.usec/USECS_PER_TICK,
	    st_p->first->hdr->network_id);
    vol[strlen(vol)] = ' ';
    if (xwrite(out->iob->fd,vol,blksize) != blksize) {
	fprintf (info, "Error writing volhdr\n");
	exit(1);
    }
    free(vol);
    return;
}
