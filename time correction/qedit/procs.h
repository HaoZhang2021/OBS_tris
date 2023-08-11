/************************************************************************/
/*  Functions in qedit.							*/
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

/*	$Id: procs.h,v 1.8 2000/10/26 13:39:39 doug Exp $ 	*/

#include "epoch.h"
#include "get_rate.h"

/************************************************************************/
/*  bounds.c		*/

int set_bound
   (ARG		*arg,		/* ptr to ARG structure.		*/
    char	*token,		/* token string.			*/
    BOUND_KEYVAL_TABLE *pt);	/* ptr to boundary keyval table.	*/

BOUND_KEYVAL bound_str_value
   (char	*str);		/* string version of bound.		*/

int typecheck_bounds
   (ARG		from,		/* start bound.				*/
    ARG		to);		/* end bound.				*/

int cvt_str_bound
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    ARG		*arg,		/* ptr to ARG structure.		*/
    int		type);		/* type of bound specified.		*/

int resolve_bounds
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* command to resolve bound for.	*/

/************************************************************************/
/*  do.c			*/

void do_help
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_find
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_verify
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_delete
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_position
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_bounds
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_display
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_add
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_set
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_bool
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_set_time
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_getpcf
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_add_time
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_add_trend
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    INT_TIME	*from_t,	/* ptr to start time for trend.		*/
    INT_TIME	*to_t);		/* ptr to end time for trend.		*/

void do_resequence
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    int		first_block_in_resequence);	/* start block number.	*/

void do_limit
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_smooth
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    DIRECTION	direction,	/* which direction to check for gaps.	*/
    int		first_block_flag);	/* 1 if first block in smooth.	*/

void do_tcorr
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    DIRECTION	direction,	/* which direction to check for gaps.	*/
    int		first_block_flag);	/* 1 if first block in smooth.	*/

void do_apply_corr 
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void do_autodisplay
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

void show_hdr_summary
   (BLKINFO	*bp);		/* ptr to block info.			*/

void show_hdr
   (BLKINFO	*bp,		/* ptr to block info.			*/
    int		hdr_type);	/* type of file.			*/

void show_stream 
   (ST_INFO	*st_p);		/* ptr to stream structure.		*/

int bool_int
   (OP		op,		/* boolean operator.			*/
    int		v1,		/* first operand.			*/
    int		v2);		/* second operand.			*/

void apply_time_correction
   (DATA_HDR	*hdr,		/* ptr to data hdr for record.		*/
    TARGET	flag);		/* flag to keep or clear corr field.	*/

void update_times 
   (DATA_HDR	*hdr);		/* ptr to data hdr for record.		*/

int are_contiguous
   (DATA_HDR	*hdr1,		/* ptr to data hdr for first record.	*/
    DATA_HDR	*hdr2);		/* ptr to data hdr for second record.	*/

void set_corr_limit
   (int		rate,		/* sample rate for data.		*/
   int		rate_mult);	/* sample rate_mult for data.		*/

/************************************************************************/
/*  execute.c			*/

int execute
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd);		/* ptr to cmd to execute.		*/

int within_bound
   (BLKINFO	*bp,		/* ptr to block info.			*/
    ARG		*lower,		/* ptr to lower arg iteration structure.*/
    ARG		*upper,		/* ptr to upper arg iteration structure.*/
    BOUND_KEYWD ub,		/* type of final bound (TO or THRU);.	*/
    DIRECTION	direction);	/* direction of movement.		*/

int find_block
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    ARG		*arg);		/* ptr to ARG structure.		*/

/************************************************************************/
/*  get_block_info.c		*/

void get_block_info
   (ST_INFO	*st_p);		/* ptr to current stream header.	*/

/************************************************************************/
/*  get_stream_info.c		*/

int get_stream_info 
   (ST_INFO	*st_p);		/* ptr to current stream header.	*/

int get_header_type 
   (char    *p,			/* ptr to buffer containing header.	*/
    int	    nr);		/* max number of bytes for header.	*/

/************************************************************************/
/*  parse_cmd.c			*/

CMD *parse_cmd
   (char	*str);		/* command string to parse.		*/

OP op_value
   (char    *str);		/* command string to get OP value for.	*/

char *op_string
   (OP	    op,			/* command value.			*/
    char    *dflt_str);		/* string for default value.		*/

TARGET target_value
   (char	*str);		/* target string.			*/

char *target_string
   (DATATYPE	target,		/* target value.			*/
    char    *dflt_str);		/* string for default value.		*/

DIRECTION direction_value
   (char	*str);		/* direction string.			*/

char *direction_string
   (DIRECTION direction,	/* direction value.			*/
    char    *dflt_str);		/* string for default value.		*/

char *datatype_string
   (DATATYPE	datatype,	/* datatype value.			*/
    char	*dflt_str);	/* string for default value.		*/

INT_TIME date_value
   (char	*str);		/* date string.				*/

char *date_string
   (INT_TIME	time,		/* date/time to convert.		*/
    char	*dflt_str);	/* default string value.		*/

char *udate_string
   (INT_TIME	time,		/* date/time to convert.		*/
    char	*dflt_str);	/* default string value.		*/

char *interval_string
   (EXT_TIME	*interval,	/* interval to convert.			*/
    char	*dflt_str);	/* default string.			*/

int cmdline_value
   (CMD		*cmd);		/* command string.			*/

int set_arg 
   (ARG		*arg,		/* ptr to argument structure.		*/
    char	*token);	/* argument string.			*/

void cmd_free
   (CMD		*cmd);		/* ptr to command structure.		*/

/************************************************************************/
/*  parse_cmdline.c		*/

int parse_cmdline 
   (int		*pargc,		/* ptr to argc.				*/
    char	***pargv);	/* ptr to argv.				*/

/************************************************************************/
/*  position_stream.c		*/

int position_stream 
   (ST_INFO	*st_p,		/* ptr to current input stream.		*/
    INT_TIME	*stime,		/* ptr to desired start time.		*/
    INT_TIME	*etime);	/* ptr to desired end time.		*/

/************************************************************************/
/*  qedit.c			*/

void dump_cmd
   (CMD		*cmd);		/* ptr to command structure.		*/

void output_prompt
   (char	*cmdname,	/* program name.			*/
    char	*prompt);	/* prompt to output (NULL -> no prompt)	*/

/************************************************************************/
/*  qio.c			*/

ST_INFO *open_stream
   (ST_INFO	*phead,		/* ptr to stream struct linked list.	*/
    char	*file,		/* filename to open.			*/
    char	*mode);		/* mode for open.			*/

ST_INFO *close_stream
   (ST_INFO	*st_p);		/* ptr to stream struct to close.	*/

int fill_buffer
   (ST_INFO	*st_p,		/* ptr to input stream struct.		*/
    int		n);		/* number of bytes desired in IOB.	*/

void write_stream
   (ST_INFO	*out);		/* ptr to output stream struct.		*/

/************************************************************************/
/*  store_block.c		*/

void store_block 
   (ST_INFO	*st_p);		/* ptr to input stream struct.		*/

/************************************************************************/
/*  write_file.c		*/

void do_write_file
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to command structure.		*/
    char	*filename);	/* output filename.			*/

int pad_out
   (ST_INFO	*out,		/* ptr to stream structure.		*/
    DATA_HDR	*ch,		/* ptr to current DATA_HDR.		*/
    int		datalen);	/* # of bytes of data.			*/

void write_file_sdr 
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out);		/* ptr to output stream structure.	*/

void write_file_drm
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out);		/* ptr to output stream structure.	*/

void write_seed_hdr
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    SDR_HDR	*sh);		/* ptr to SEED Data Header buffer.	*/

void write_vol
   (ST_INFO	*st_p,		/* ptr to input stream structure.	*/
    ST_INFO	*out);		/* ptr to output stream structure.	*/

