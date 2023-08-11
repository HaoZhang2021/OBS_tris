/************************************************************************/
/*  Routines to execute each individual command.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2004 The Regents of the University of California.
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
static char sccsid[] = "$Id: do.c,v 1.23 2005/12/14 02:16:33 doug Exp $ ";
#endif

#include <stdio.h>
#include <math.h>

#include "qlib2.h"
#define	MAXODHSTRLEN	8192

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"
#include "get_rate.h"

#define UNKNOWN_TARGET(cmd) { \
    fprintf (info, "Unknown target: %s\n", \
	     target_string((DATATYPE)cmd->target, NULL_STR)); \
    ++no_auto_display; }

#define UNKNOWN_DIRECTION(direction) { \
    fprintf (info, "Unknown direction: %d\n", (int)direction); \
    ++no_auto_display; }

#define INVALID_ARG0_STR(cmd) { \
    fprintf (info, "Invalid argument for %s %s: %s\n", \
	     op_string((OP)cmd->op, NULL_STR), \
	     target_string((DATATYPE)cmd->target, NULL_STR), \
	     cmd->arg[0].argv.str); \
    ++no_auto_display; }

#define VALIDATE_CURRENT_RECORD(st_p) \
    if (st_p->cur == NULL) { \
	fprintf (info, "Unable to execute - no current block\n"); \
	return; } \

#define	BLANK_PROXY(str) \
    (str != NULL && str[0] == '-')

#define	DEFAULT_CORR_LIMIT	10
#define	DEFAULT_MINGAP		10
#define	DEFAULT_MAXGAP		2147483647
static int corr_limit =	DEFAULT_CORR_LIMIT;
static int mingap =	DEFAULT_MINGAP;    
static int maxgap =	DEFAULT_MAXGAP;

static CLOCKRATE *rate_head;

/************************************************************************/
/*  Do functions that actually perform the work for the commands.	*/
/************************************************************************/
void do_help
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    CMD_TABLE	*ct = cmd_table;
    OP_TABLE	*ot = op_table;
    char	*str;
    char	*specific;
    OP		this_op;
    int		cmd_found = FALSE;

    /*	Command aliases are defined as multiple strings in op_table	*/
    /*	that have the same OP value.  However, there will be only a	*/
    /*	a single OP entry for it in cmd_table, since it contains the OP	*/
    /*	rather than the string.						*/
    /*	Therefore, we must search the op_table for the command, and	*/
    /*	when we find it, we must then find the corresponding OP entry	*/
    /*	in the cmd_table.						*/

    if (debug(DEBUG_DO)) fprintf (info, "help command\n");
    ++no_auto_display;

    specific = (cmd->arg[0].argt == STR_TYPE) ? cmd->arg[0].argv.str : NULL;
    for (ot=op_table; ot->op != NO_OP; ot++) {
	if ((! specific) || MATCH_STR(specific,ot->string)) {
	    this_op = ot->op;
	    for (ct=cmd_table; ct->op != NO_OP; ct++) {
		if (MATCH_VAL (this_op, ct->op)) {
		    fprintf (info, " %s", ot->string);

		    str = target_string((DATATYPE)ct->target, "");
		    if (str == NULL) str = "";
		    fprintf (info, " %s", str);

		    str = datatype_string(ct->arg_0, "");
		    if (str == NULL) str = "";
		    fprintf (info, " %s", str);

		    str = datatype_string(ct->arg_1, "");
		    if (str == NULL) str = "";
		    fprintf (info, " %s", str);

		    fprintf (info, "\t- %s", ct->description);
		    fprintf (info, "\n");
		    ++cmd_found;
		}
	    }
	}
    }
    if (cmd_found) {
	BOUND_KEYVAL_TABLE *bt;
	int use_comma = 0;
	fprintf (info, "\n");
	fprintf (info, "NOTE: command be preceeded by: from <date|record_#|position> to|thru <date|record_#|position>\n");
	fprintf (info, "Position is one of the following strings:");
	for (bt = bound_keyval_table; bt->key != NO_KEYVAL; bt++) {
	    fprintf (info, "%s%s", (use_comma++) ? ", " : " ", bt->string);
	}
	fprintf (info, "\n");
    }
    else fprintf (info, "Command \"%s\" not found.\n", specific);
}

void do_find
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    BLKINFO	*initial_cur, *initial_prevpos;
    char	*target_str;
    int		seconds, usecs, sample_usecs, ticks;
    
    if (debug(DEBUG_DO)) fprintf (info, "find command\n");
    initial_cur = st_p->cur;
    initial_prevpos = st_p->prevpos;
    switch (cmd->target) {
	case TEAR:
	    /* Tear values are still represented as ticks.		*/
	    if (st_p->cur && st_p->cur->hdr) {
		time_interval2 (1, st_p->cur->hdr->sample_rate, 
			       st_p->cur->hdr->sample_rate_mult, 
			       &seconds, &usecs);
		sample_usecs = (seconds * USECS_PER_SEC) + usecs;
	    }
	    /* Do no wrap to top - causes problems if you iterate over time, */
	    /* and there is a time tear before the last record. */
	    /* if (st_p->cur == st_p->last) st_p->cur = st_p->first; */
	    break;
	case RECORD:
	case BLOCK:
	case HDRTIME:
	case BEGTIME:
	case TIME:
	    st_p->cur = st_p->first;
	    break;
	case SEQNO: break;
	default:    
	    UNKNOWN_TARGET(cmd); return;
    }
    st_p->prevpos = st_p->cur;
    while (st_p->cur != NULL) {
	switch (cmd->target) {
	    case BLOCK:
	    case RECORD:
		if (st_p->cur->blk_no == cmd->arg[0].argv.ival) 
		return;
		break;
	    case SEQNO:
		if (st_p->cur->hdr->seq_no == cmd->arg[0].argv.ival) 
		return;
		break;
	    case HDRTIME:
		if (tdiff (st_p->cur->hdr->hdrtime, *cmd->arg[0].argv.time) == 0.)
		    return;
		break;
	    case BEGTIME:
		if (tdiff (st_p->cur->hdr->begtime, *cmd->arg[0].argv.time) == 0.)
		    return;
		break;
	    case TEAR:
		/* Tear values are still represented as ticks.		*/
		if (st_p->cur->next && 
		    abs(ticks=(int)((tdiff(st_p->cur->next->hdr->begtime,
					   st_p->cur->hdr->endtime) - sample_usecs)
				    / USECS_PER_TICK)) >= cmd->arg[0].argv.ival) {
		    st_p->cur = st_p->cur->next;
		    fprintf (stderr, "Apparent tear = %d ticks\n", ticks);
		    return;
		}
		break;
	    case TIME:
		time_interval2 (1, st_p->cur->hdr->sample_rate, 
			       st_p->cur->hdr->sample_rate_mult, 
			       &seconds, &usecs);
		if (tdiff (*cmd->arg[0].argv.time, st_p->cur->hdr->begtime) >= 0 &&
		    tdiff (add_time(st_p->cur->hdr->endtime, seconds, usecs),
			      *cmd->arg[0].argv.time) > 0)
		    return;
		/*  Also match if we are the first block AFTER the time.    */
		if (tdiff (*cmd->arg[0].argv.time, st_p->cur->hdr->begtime) <= 0)
		    return;
		break;
	    default:
		UNKNOWN_TARGET(cmd); st_p->prevpos = initial_prevpos; return;
	}
	st_p->cur = st_p->cur->next;
    }
    if (st_p->cur == NULL) {
	fprintf (info, "NOT FOUND: %s %s\n", 
		 target_string ((DATATYPE)cmd->target, NULL_STR), 
		 arg_string (&cmd->arg[0], NULL_STR));
	st_p->cur = initial_cur;
	st_p->prevpos = initial_prevpos;
	++no_auto_display;
    }
}

void do_verify
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    BLKINFO	*initial_cur, *initial_prevpos;
    char	*target_str;
    char	*v_station, *v_location, *v_channel, *v_network, v_record_type;
    char	*arg0_str;
    if (debug(DEBUG_DO)) fprintf (info, "verify command\n");
    initial_cur = st_p->cur;
    initial_prevpos = st_p->prevpos;
    /* Use the convention of "-" or "--" for a blank string. */
    switch (cmd->target) {
	case ALL:
	    if (st_p->cur != NULL) {
		v_station = st_p->cur->hdr->station_id;
		v_location = st_p->cur->hdr->location_id;
		v_channel = st_p->cur->hdr->channel_id;
		v_network = st_p->cur->hdr->network_id;
		v_record_type = st_p->cur->hdr->record_type;
	    }
	    break;
	case STATION:
	    if (cmd->arg[0].argt == STR_TYPE) v_station = cmd->arg[0].argv.str;
	    else if (st_p->first != NULL) v_station = st_p->first->hdr->station_id;
	    break;
	case NETWORK:
	    arg0_str = (BLANK_PROXY(cmd->arg[0].argv.str)) ? "" : cmd->arg[0].argv.str;
	    if (cmd->arg[0].argt == STR_TYPE) v_network = arg0_str;
	    else if (st_p->first != NULL) v_network = st_p->first->hdr->network_id;
	    break;
	case CHANNEL:
	    if (cmd->arg[0].argt == STR_TYPE) v_channel = cmd->arg[0].argv.str;
	    else if (st_p->first != NULL) v_channel = st_p->first->hdr->channel_id;
	    break;
	case LOCATION:
	    arg0_str = (BLANK_PROXY(cmd->arg[0].argv.str)) ? "" : cmd->arg[0].argv.str;
	    if (cmd->arg[0].argt == STR_TYPE) v_location = arg0_str;
	    else if (st_p->first != NULL) v_location = st_p->first->hdr->location_id;
	    break;
	case RECORDTYPE:
	    if (cmd->arg[0].argt == STR_TYPE) v_record_type = cmd->arg[0].argv.str[0];
	    else if (st_p->first != NULL) v_record_type = st_p->first->hdr->record_type;
	    break;
	default:    
	    UNKNOWN_TARGET(cmd); return;
    }
    st_p->prevpos = st_p->cur;
    st_p->cur = st_p->first;
    while (st_p->cur != NULL) {
	switch (cmd->target) {
	    case ALL:
	    case STATION:
		if (strcmp(v_station, st_p->cur->hdr->station_id) != 0) {
		    fprintf (info, "Station mismatch at block %d -- expecting \"%s\", found \"%s\"\n",
			     st_p->cur->blk_no, v_station, st_p->cur->hdr->station_id);	
		    return;
		}
		if (cmd->target == STATION) break;
	    case NETWORK:
		if (strcmp(v_network, st_p->cur->hdr->network_id) != 0) {
		    fprintf (info, "Network mismatch at block %d -- expecting \"%s\", found \"%s\"\n",
			     st_p->cur->blk_no, v_network, st_p->cur->hdr->network_id);	
		    return;
		}
		if (cmd->target == NETWORK) break;
	    case CHANNEL:
		if (strcmp(v_channel, st_p->cur->hdr->channel_id) != 0) {
		    fprintf (info, "Channel mismatch at block %d -- expecting \"%s\", found \"%s\"\n",
			     st_p->cur->blk_no, v_channel, st_p->cur->hdr->channel_id);	
		    return;
		}
		if (cmd->target == CHANNEL) break;
	    case LOCATION:
		if (strcmp(v_location, st_p->cur->hdr->location_id) != 0) {
		    fprintf (info, "Location mismatch at block %d -- expecting \"%s\", found \"%s\"\n",
			     st_p->cur->blk_no, v_location, st_p->cur->hdr->location_id);	
		    return;
		}
		if (cmd->target == LOCATION) break;
	    case RECORDTYPE:
		if (v_record_type != st_p->cur->hdr->record_type) {
		    fprintf (info, "Rectype mismatch at block %d -- expecting \"%c\", found \"%c\"\n",
			     st_p->cur->blk_no, v_record_type, st_p->cur->hdr->record_type);	
		    return;
		}
		if (cmd->target == RECORDTYPE) break;
	    break;  /* for case ALL */
	    default:
		UNKNOWN_TARGET(cmd); st_p->prevpos = initial_prevpos; return;
	}
	st_p->cur = st_p->cur->next;
    }
    st_p->cur = initial_cur;
    st_p->prevpos = initial_prevpos;
    ++no_auto_display;
}

void do_delete
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    BLKINFO	*p;
    int		n;
    if (debug(DEBUG_DO)) fprintf (info, "delete command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    switch (cmd->target) {
	case NO_TARGET:
	    n = (cmd->arg[0].argt == INT_TYPE) ? cmd->arg[0].argv.ival : 1;
	    while (n-- > 0) {
		p = st_p->cur;
		if (p == NULL) {
		    fprintf (info, "deleted to EOF.\n");
		    break;
		}
		/*  Remove block from linked list.	*/
		/*	If block was first or last block, adjust ptrs.	*/
		/*	Point to next block, unless this was last.	*/
		if (p->prev != NULL) p->prev->next = p->next;
		if (p->next != NULL) p->next->prev = p->prev;
		if (st_p->first == p) st_p->first = p->next;
		if (st_p->last == p) {
		    st_p->last = p->prev;
		    n = 0;
		    fprintf (info, "deleted to EOF.\n");
		}
		st_p->cur = (p->next != NULL) ? p->next : p->prev;

		/*	Deallocate block, header, and data.		*/
		free (p->data);
		free_data_hdr (p->hdr);
		free (p);
		--st_p->num_blocks;
	    }
	    if (st_p->cur == NULL) st_p->cur = st_p->last;
	    return;
	default:	    
	    fprintf (info, "Unknown delete command: %s\n", op_string(cmd->op, NULL_STR));
    }
}

void do_position
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    BLKINFO	*initial_prevpos;
    int		n = (cmd->arg[0].argt == INT_TYPE) ? cmd->arg[0].argv.ival : 1;
    if (debug(DEBUG_DO)) fprintf (info, "position command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    /*	If no current position, start at first block.		    */
    if (st_p->cur == NULL) st_p->cur = st_p->first;
    if (st_p->prevpos == NULL) st_p->prevpos = st_p->first;
    initial_prevpos = st_p->prevpos;
    st_p->prevpos - st_p->cur;
    switch (cmd->op) {
	case FIRST:
	    st_p->cur = st_p->first;
	    break;
	case LAST:
	    st_p->cur = st_p->last;
	    break;
	case FORWARD:
	    while (st_p->cur != NULL && n-- > 0) 
		st_p->cur = st_p->cur->next;
	    if (st_p->cur == NULL) {
		fprintf (info, "Unable to position past end of stream.\n");
		st_p->cur = st_p->last;
	    }
	    break;
	case BACK:
	    while (st_p->cur != NULL && n-- > 0) 
		st_p->cur = st_p->cur->prev;
	    if (st_p->cur == NULL) {
		fprintf (info, "Unable to position before start of stream.\n");
		st_p->cur = st_p->first;
	    }
	    break;
	default:	    
	    fprintf (info, "Unknown position command: %s\n", op_string(cmd->op, NULL_STR));
	    st_p->prevpos = initial_prevpos;
    }
}

void do_bounds
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "bounds command\n");
    switch (cmd->target) {
	case ON:    extended_bounds = 0; break;
	case OFF:   extended_bounds = 1; break;
	default:    UNKNOWN_TARGET(cmd);
    }
}

void do_display
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "display command\n");
    ++no_auto_display;
    switch (cmd->target) {
	case NO_TARGET:
	case HEADER:
	    VALIDATE_CURRENT_RECORD(st_p);
	    if (st_p->cur != NULL) {
		show_hdr(st_p->cur, st_p->hdr_type);
	    }
	    else fprintf (info, "No current block\n");
	    break;
	case STREAM:
	    show_stream(st_p);
	    break;
	default:    UNKNOWN_TARGET(cmd);
    }
}

void do_add
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "add command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    switch (cmd->target) {
	case NSAMPLES:
	    st_p->cur->hdr->num_samples += cmd->arg[0].argv.ival;
	    fprintf (info, "Warning -- this does NOT change info in data block.");
	    break;
	case RATE:
	    st_p->cur->hdr->sample_rate += cmd->arg[0].argv.ival;
	    break;
	case RATE_MULT:
	    st_p->cur->hdr->sample_rate_mult += cmd->arg[0].argv.ival;
	    break;
	case CORRECTION:
	    st_p->cur->hdr->num_ticks_correction += cmd->arg[0].argv.ival;
	    update_times (st_p->cur->hdr);
	    break;
	case X0:
	    st_p->cur->hdr->x0 += cmd->arg[0].argv.hval;
	    ((FRAME *)(st_p->cur->data))->w[0].fw = st_p->cur->hdr->x0;
	    fprintf (info, "Warning -- this does NOT change difference value in data block.\n");
	    break;
	case XN:
	    st_p->cur->hdr->xn += cmd->arg[0].argv.ival;
	    ((FRAME *)(st_p->cur->data))->w[1].fw = st_p->cur->hdr->xn;
	    fprintf (info, "Warning -- this does NOT change difference value in data block.\n");
	    break;
	default:    
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
    return;
}

void do_set
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    char	*arg0_str;
    if (debug(DEBUG_DO)) fprintf (info, "set command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    /* Use the convention of "-" or "--" for a blank string. */
    switch (cmd->target) {
	case STATION:
	    strncpy(st_p->cur->hdr->station_id, cmd->arg[0].argv.str, DH_STATION_LEN);
	    st_p->cur->hdr->station_id[DH_STATION_LEN] = '\0';
	    break;
	case NETWORK:
	    arg0_str = (BLANK_PROXY(cmd->arg[0].argv.str)) ? "" : cmd->arg[0].argv.str;
	    strncpy(st_p->cur->hdr->network_id, arg0_str, DH_NETWORK_LEN);
	    st_p->cur->hdr->network_id[DH_NETWORK_LEN] = '\0';
	    break;
	case CHANNEL:
	    strncpy(st_p->cur->hdr->channel_id, cmd->arg[0].argv.str, DH_CHANNEL_LEN);
	    st_p->cur->hdr->channel_id[DH_CHANNEL_LEN] = '\0';
	    break;
	case LOCATION:
	    arg0_str = (BLANK_PROXY(cmd->arg[0].argv.str)) ? "" : cmd->arg[0].argv.str;
	    strncpy(st_p->cur->hdr->location_id, arg0_str, DH_LOCATION_LEN);
	    st_p->cur->hdr->location_id[DH_LOCATION_LEN] = '\0';
	    break;
	case RECORDTYPE:
	    if (is_data_hdr_ind(cmd->arg[0].argv.str[0])) 
		st_p->cur->hdr->record_type = cmd->arg[0].argv.str[0];
	    else INVALID_ARG0_STR(cmd);
	    break;
	case NSAMPLES:
	    st_p->cur->hdr->num_samples = cmd->arg[0].argv.ival;
	    fprintf (info, "Warning -- this does NOT change info in data block.");
	    break;
	case RATE:
	    st_p->cur->hdr->sample_rate = cmd->arg[0].argv.ival;
	    break;
	case RATE_MULT:
	    st_p->cur->hdr->sample_rate_mult = cmd->arg[0].argv.ival;
	    break;
	case SEQNO:
	    st_p->cur->hdr->seq_no = cmd->arg[0].argv.ival;
	    break;
	case CORRECTION:
	    st_p->cur->hdr->num_ticks_correction = cmd->arg[0].argv.ival;
	    update_times (st_p->cur->hdr);
	    break;
	case AFLAGS:
	    st_p->cur->hdr->activity_flags = cmd->arg[0].argv.hval;
	    break;
	case IOFLAGS:
	    st_p->cur->hdr->io_flags = cmd->arg[0].argv.hval;
	    break;
	case QFLAGS:
	    st_p->cur->hdr->data_quality_flags = cmd->arg[0].argv.hval;
	    break;
	case X0:
	    st_p->cur->hdr->x0 = cmd->arg[0].argv.hval;
	    ((FRAME *)(st_p->cur->data))->w[0].fw = st_p->cur->hdr->x0;
	    fprintf (info, "Warning -- this does NOT change difference value in data block.\n");
	    break;
	case XN:
	    st_p->cur->hdr->xn = cmd->arg[0].argv.ival;
	    ((FRAME *)(st_p->cur->data))->w[1].fw = st_p->cur->hdr->xn;
	    fprintf (info, "Warning -- this does NOT change difference value in data block.\n");
	    break;
	default:    
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
    return;
}

void do_bool
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "boolean operator command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    switch (cmd->target) {
	case AFLAGS:
	    st_p->cur->hdr->activity_flags = 
		bool_int (cmd->op, (int)st_p->cur->hdr->activity_flags, cmd->arg[0].argv.ival);
	    break;
	case IOFLAGS:
	    st_p->cur->hdr->io_flags = 
		bool_int (cmd->op, (int)st_p->cur->hdr->io_flags, cmd->arg[0].argv.ival);
	    break;
	case QFLAGS:
	    st_p->cur->hdr->data_quality_flags = 
		bool_int (cmd->op, (int)st_p->cur->hdr->data_quality_flags, cmd->arg[0].argv.ival);
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
    return;
}

void do_set_time
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    INT_TIME	newtime;
    int		incr;
    if (debug(DEBUG_DO)) fprintf (info, "set_time command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    switch (cmd->target) {
	case HDRTIME:
	    st_p->cur->hdr->hdrtime = *cmd->arg[0].argv.time;
	    update_times (st_p->cur->hdr);
	    break;
	case CORRECTION:
	    newtime = add_interval(st_p->cur->hdr->hdrtime, *cmd->arg[0].argv.interval);
	    incr = (int)tdiff (newtime, st_p->cur->hdr->hdrtime);
	    st_p->cur->hdr->num_ticks_correction = incr;
	    /* Clear ACTIVITY_TIME_CORR_APPLIED, since correction is	*/
	    /* not included in the hdrtime.				*/
	    st_p->cur->hdr->activity_flags &= ~ACTIVITY_TIME_CORR_APPLIED;
	    update_times (st_p->cur->hdr);
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
    return;
}

void do_getpcf
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
	rate_head = get_rate_list(&(*cmd->arg[0].argv.str), 0, 0);
}

void do_add_time
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    INT_TIME	newtime;
    int		incr;
    double epoch;
    double correction;
    CLOCKRATE *entry;
    if (debug(DEBUG_DO)) fprintf (info, "add_time command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    switch (cmd->target) {
	case HDRTIME:
	    st_p->cur->hdr->hdrtime = add_interval(st_p->cur->hdr->hdrtime, 
						   *cmd->arg[0].argv.interval);
	    update_times (st_p->cur->hdr);
	    break;
	case PCF:
		/* inserts time correction into header but nothing more */
		if (rate_head == NULL) {
			fprintf(stderr, "No PCF file read in yet!\n"); 
			break;
		}
		/* convert header time to epoch */
		epoch = (double) unix_time_from_int_time(st_p->cur->hdr->hdrtime) +
			st_p->cur->hdr->hdrtime.usec/USECS_PER_SEC;
		/* get entry corresponding to epoch */
		entry = get_rate_entry(rate_head, epoch, 0);
		if (entry == NULL) {
			fprintf(stderr, "No Correction Found\n");
			break;
		}
	    	st_p->cur->hdr->num_ticks_correction += 
		    get_total_shift(entry, epoch, &correction) * TICKS_PER_MSEC;
	    break;
	case CORRECTION:
	    newtime = add_interval(st_p->cur->hdr->begtime, *cmd->arg[0].argv.interval);
	    incr = (int)(tdiff (newtime, st_p->cur->hdr->begtime) / USECS_PER_TICK);
	    st_p->cur->hdr->num_ticks_correction += incr;
	    update_times (st_p->cur->hdr);
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
}

void do_add_trend
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    INT_TIME	*from_t,	/* ptr to start time for trend.		*/
    INT_TIME	*to_t)		/* ptr to end time for trend.		*/
{
    INT_TIME	it1;		/* new initial time for span.		*/
    INT_TIME	it2;		/* new end time for span.		*/
    double	init_offset;	/* correction at beginning of span.	*/
    double	end_offset;	/* correction at end of span.		*/
    double	corr_length;	/* total length of span.		*/
    double	slope;		/* slope of time correction over span.	*/
    double	delta;		/* diff betw. init span and this block.	*/
    double	dcorr;		/* time correction for this block.	*/

    if (debug(DEBUG_DO)) fprintf (info, "add_trend command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    if (from_t == NULL || to_t == NULL) {
	fprintf (info, "add_trend invalid - must have iteration\n");
	return;
    }

    /* Compute initial offset and slope of trend.	*/
    it1 = add_interval (*from_t, *cmd->arg[0].argv.interval);
    it2 = add_interval (*to_t, *cmd->arg[1].argv.interval);
    init_offset = tdiff(it1, *from_t);
    end_offset = tdiff(it2, *to_t);
    corr_length = tdiff (*to_t, *from_t);
    if (corr_length <= 0.) {
	fprintf (info, "Error - Timespan over which trend is added must be positive.\n");
	return;
    }
    slope = (end_offset - init_offset) / corr_length;

    /* NOTE:  No check is made to determine whether the bit field for	*/
    /* time_correction_added is set or not.  Just blindly add the	*/
    /* appropriate value to the hdr or correction field, and then 	*/
    /* update the begtime and endtime.					*/
    switch (cmd->target) {
	case HDRTIME:
	    delta = tdiff(st_p->cur->hdr->begtime,*from_t);
	    /* Currently round to corr_limit ticks. */
	    dcorr = (double)roundoff ((init_offset + (delta * slope)) / (corr_limit * USECS_PER_TICK))
		* (corr_limit * USECS_PER_TICK);
	    st_p->cur->hdr->hdrtime = add_dtime(st_p->cur->hdr->hdrtime, dcorr);
	    update_times (st_p->cur->hdr);
	    break;
	case CORRECTION:
	    delta = tdiff(st_p->cur->hdr->begtime,*from_t);
	    /* Currently round to corr_limit ticks. */
	    dcorr = (double)roundoff ((init_offset + (delta * slope)) / (corr_limit * USECS_PER_TICK))
		* (corr_limit * USECS_PER_TICK);
	    st_p->cur->hdr->num_ticks_correction += (int)(dcorr / USECS_PER_TICK);
	    update_times (st_p->cur->hdr);
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
}

void do_resequence
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    int		first_block_in_resequence)	/* start block number.	*/
{
    BLKINFO	*p = st_p->cur;
    if (debug(DEBUG_DO)) fprintf (info, "resequence command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    if (first_block_in_resequence) return;
    /*	Resequence based on previous block.				*/
    /*	If this block is "contiguous" with previous block, then use the	*/
    /*	next sequence number for this block.  Otherwise, use 1.		*/
    /*	By definition, there MUST be a previous block, since this is	*/
    /*	not the first block in the resequence interval.			*/
    p->hdr->seq_no = (are_contiguous (p->prev->hdr, p->hdr)) ?
	p->prev->hdr->seq_no + 1 : 1;
}

void do_limit
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    int		n;
    switch (cmd->target) {
	case CORRECTION:
	    n = (cmd->arg[0].argt == INT_TYPE) ? cmd->arg[0].argv.ival : DEFAULT_CORR_LIMIT;
	    if (n <= 0) {
		fprintf (info, "Invalid CORR limit - must be > 0.\n");
		return;
	    }
	    corr_limit = n;
	    break;
	case MINGAP:
	    n = (cmd->arg[0].argt == INT_TYPE) ? cmd->arg[0].argv.ival : DEFAULT_MINGAP;
	    if (n <= 0) {
		fprintf (info, "Invalid MINGAP limit - must be > 0.\n");
		return;
	    }
	    mingap = n;
	    break;
	case MAXGAP:
	    n = (cmd->arg[0].argt == INT_TYPE) ? cmd->arg[0].argv.ival : DEFAULT_MAXGAP;
	    if (n <= 0) {
		fprintf (info, "Invalid MAXGAP limit - must be > 0.\n");
		return;
	    }
	    maxgap = n;
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
}

void do_smooth
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    DIRECTION	direction,	/* which direction to check for gaps.	*/
    int		first_block_flag)	/* 1 if first block in smooth.	*/
{
    BLKINFO	*p = st_p->cur;
    int		seconds, usecs, sample_usecs, tickcorr;
    double	dcorr;
    INT_TIME	prevt, currt, nextt;

    if (debug(DEBUG_DO)) fprintf (info, "smooth command\n");
    VALIDATE_CURRENT_RECORD(st_p);
    if (first_block_flag) return;
    time_interval2 (1, st_p->cur->hdr->sample_rate, 
		    st_p->cur->hdr->sample_rate_mult,
		    &seconds, &usecs);
    sample_usecs = (seconds * USECS_PER_SEC) + usecs;
    if (debug(DEBUG_DETAIL)) {
	fprintf (info, "Current time for blk_no %d is %s\n", st_p->cur->blk_no,
		 date_string(st_p->cur->hdr->begtime, NULL_STR));
    }
    switch (direction) {
	case FORWARD_DIR:
	    /*	Smooth time with previous block.			*/
	    /*	If this block is not "contiguous" with previous block, 	*/
	    /*  calculate the time adjustment necessary to make it	*/
	    /*  contiguous and add that time adjustment to either the	*/
	    /*  hdrtime or the time correction field.			*/
	    if (st_p->cur->prev == NULL) return;
	    prevt = st_p->cur->prev->hdr->endtime;
	    currt = st_p->cur->hdr->begtime;
	    /* Remove precision less than 1 tick. */
	    prevt.usec = (prevt.usec/USECS_PER_TICK) * USECS_PER_TICK;
	    currt.usec = (currt.usec/USECS_PER_TICK) * USECS_PER_TICK;
	    dcorr = tdiff(prevt,currt) + sample_usecs;
	    if (debug(DEBUG_DETAIL)) {
		fprintf (info, "tdiff = %.0lf usecs between blk_no %d and %d\n",
			 -1*dcorr, st_p->cur->prev->blk_no, st_p->cur->blk_no);
	    }
	    break;
	case REVERSE_DIR:
	    /*	Smooth time with next block.			*/
	    /*	If this block is not "contiguous" with next block, 	*/
	    /*  calculate the time adjustment necessary to make it	*/
	    /*  contiguous and add that time adjustment to either the	*/
	    /*  hdrtime or the time correction field.			*/
	    if (st_p->cur->next == NULL) return;
	    nextt = st_p->cur->next->hdr->begtime;
	    currt = st_p->cur->hdr->endtime;
	    /* Remove precision less than 1 tick. */
	    nextt.usec = (nextt.usec/USECS_PER_TICK) * USECS_PER_TICK;
	    currt.usec = (currt.usec/USECS_PER_TICK) * USECS_PER_TICK;
	    dcorr = tdiff(nextt,currt) - sample_usecs;
	    if (debug(DEBUG_DETAIL)) {
		fprintf (info, "tdiff = %.0lf usecs between blocks %d and %d\n",
			 dcorr, st_p->cur->blk_no, st_p->cur->next->blk_no);
	    }
	    break;
	default:
	    UNKNOWN_DIRECTION(direction); return;
	    break;
    }
    /* Do not perform smoothing if gap size is outside desired bounds.	*/
    if (fabs(dcorr) < ((double)mingap)*USECS_PER_TICK || fabs(dcorr) > ((double)maxgap)*USECS_PER_TICK) {
	if (debug(DEBUG_DETAIL)) {
	    fprintf (info, "No time correction applied (mingap = %d ticks, maxgap=%d ticks)\n",
		     mingap, maxgap);
	}
	return;
    }
    /* Currently round to corr_limit ticks. */
    /* We cannot correct in any finer resolution than ticks.		*/
    dcorr = (double)(roundoff (dcorr / (corr_limit * USECS_PER_TICK))) 
	* (corr_limit * USECS_PER_TICK);
    switch (cmd->target) {
	case HDRTIME:   
	    st_p->cur->hdr->hdrtime = add_dtime(st_p->cur->hdr->hdrtime, dcorr);
	    update_times (st_p->cur->hdr);
	    if (debug(DEBUG_DETAIL)) {
		fprintf (info, "Applied time correction of %.0lf usecs to hdrtime for blk_no = %d.\n",
			 dcorr, st_p->cur->blk_no);
	    }
	    break;
	case CORRECTION:   
	    tickcorr = (int)(dcorr / USECS_PER_TICK);
	    st_p->cur->hdr->num_ticks_correction += tickcorr;
	    update_times (st_p->cur->hdr);
	    if (debug(DEBUG_DETAIL)) {
		fprintf (info, "Applied time correction of %d ticks to timecorr for blk_no = %d.\n",
			 tickcorr, st_p->cur->blk_no);
	    }
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
    if (debug(DEBUG_DETAIL)) {
	fprintf (info, "New time for blk_no %d is %s\n", st_p->cur->blk_no,
		 date_string(st_p->cur->hdr->begtime, NULL_STR));
    }
}

void do_tcorr
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    DIRECTION	direction,	/* which direction to check for gaps.	*/
    int		first_block_flag)	/* 1 if first block in smooth.	*/
{
    if (debug(DEBUG_DO)) fprintf (info, "tcorr command\n");
    if (first_block_flag) return;
    do_smooth (st_p, cmd, direction, 0);
}

void do_apply_corr 
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "apply_corr command\n");
    switch (cmd->target) {
	case KEEP_FIELD:   
	case CLEAR_FIELD:   
	    apply_time_correction (st_p->cur->hdr, cmd->target);
	    break;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
}

void do_autodisplay
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    if (debug(DEBUG_DO)) fprintf (info, "autodisplay command\n");
    switch (cmd->target) {
	case OFF:   
	case ON:    
	case ALL:   
	    auto_display = cmd->target; return;
	default:
	    UNKNOWN_TARGET(cmd); return;
	    break;
    }
}

void do_volhdr
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    int n;
    if (debug(DEBUG_DO)) fprintf (info, "volhdr command\n");
    if (st_p->hdr_type == DRM_HDR_TYPE) {
	fprintf (stderr, "Unable to delete or create DRM volume header.");
	return;
    }
    switch (cmd->target) {
	case OFF:    
	    if (st_p->vfh) free(st_p->vfh);
	    st_p->vfh = NULL;
	    break;
	case ON:   
	    n = st_p->blksize * sizeof(char);
	    st_p->vfh = (char *)malloc(n);
	    if (st_p->vfh == NULL) {
		fprintf (stderr, "Unable to malloc volhdr for blksize %d\n", st_p->blksize);
		break;
	    }
	    memset ((void *)st_p->vfh, 0, n);
	    /* Contents of the volume will be created on output. */
	    break;
	default:    UNKNOWN_TARGET(cmd);
    }
}

/************************************************************************/
/*  b1000_format_str:							*/
/*	Return a string for the specified b1000 format.			*/
/************************************************************************/
char *
b1000_format_str(format)
    int format;
{
    switch (format) {
      case INT_16:			return ("INT_16");
      case INT_24:			return ("INT_24");  
      case INT_32:			return ("INT_32");
      case IEEE_FP_SP:			return ("IEEE_FP_SP");
      case IEEE_FP_DP:			return ("IEEE_FP_DP");
      case STEIM1:			return ("STEIM1");
      case STEIM2:			return ("STEIM2");
      case GEOSCOPE_MULTIPLEX_24:	return ("GEOSCOPE_MULTIPLEX_24");
      case GEOSCOPE_MULTIPLEX_16_GR_3:	return ("GEOSCOPE_MULTIPLEX_16_GR_3");
      case GEOSCOPE_MULTIPLEX_16_GR_4:	return ("GEOSCOPE_MULTIPLEX_16_GR_4");
      case USNN:			return ("USNN");
      case CDSN:			return ("CDSN");
      case GRAEFENBERG_16:		return ("GRAEFENBERG_16");
      case IPG_STRASBOURG_16:		return ("IPG_STRASBOURG_16");
      case SRO:				return ("SRO");
      case HGLP:			return ("HGLP");
      case DWWSSN_GR:			return ("DWWSSN_GR");
      case RSTN_16_GR:			return ("RSTN_16_GR");
      default:				return ("unknown");
    }
}


/************************************************************************/
/*  show_hdr_summary:							*/
/*	Print the specified block summary information.			*/
/************************************************************************/
void show_hdr_summary
   (BLKINFO	*bp)		/* ptr to block info.			*/
{
    DATA_HDR	*hdr;
    if (hdr == NULL) {
	fprintf (info, "(none)\n");
	return;
    }
    hdr = bp->hdr;
    fprintf (info, "recno = %d  ", bp->blk_no);
    fprintf (info, "seqno = %d  ", hdr->seq_no);
    fprintf (info, "begtime = %s\n", udate_string(hdr->begtime, NULL_STR));
}

/************************************************************************/
/*  show_hdr:								*/
/*	Print the specified block header information.			*/
/************************************************************************/
void show_hdr
   (BLKINFO	*bp,		/* ptr to block info.			*/
    int		hdr_type)	/* type of file.			*/
{
    int		bl;
    int		steim_comp;
    BS		    *bs;		/* ptr to blockette structure.	*/
    BLOCKETTE_HDR   *bh;		/* ptr to blockette hdr.	*/
    DATA_HDR	*hdr = bp->hdr;
    fprintf (info, "recno = %d\n", bp->blk_no);
    fprintf (info, "seqno = %d\n", hdr->seq_no);
    fprintf (info, "rectype  = %c\n", hdr->record_type);
    fprintf (info, "station  = %s\n", hdr->station_id);
    fprintf (info, "location = %s\n", hdr->location_id);
    fprintf (info, "channel  = %s\n", hdr->channel_id);
    fprintf (info, "network  = %s\n", hdr->network_id);
    fprintf (info, "hdrtime  = %s\n", date_string(hdr->hdrtime, NULL_STR));
    fprintf (info, "begtime  = %s\n", udate_string(hdr->begtime, NULL_STR));
    fprintf (info, "endtime  = %s\n", udate_string(hdr->endtime, NULL_STR));
    fprintf (info, "sample_rate = %d\n", hdr->sample_rate);
    fprintf (info, "sample_rate_mult = %d\n", hdr->sample_rate_mult);
    fprintf (info, "activity_flags = 0x%02x\n", hdr->activity_flags);
    fprintf (info, "io_flags = 0x%02x\n", hdr->io_flags);
    fprintf (info, "data_quality_flags = 0x%02x\n", hdr->data_quality_flags);
    fprintf (info, "num_ticks_correction = %d\n", hdr->num_ticks_correction);
    fprintf (info, "num_blockettes = %d\n", hdr->num_blockettes);
    fprintf (info, "first_blockette = %d\n", hdr->first_blockette);
    fprintf (info, "num_samples = %d\n", hdr->num_samples);
    fprintf (info, "first_data = %d\n", hdr->first_data);
    fprintf (info, "blksize = %d\n", hdr->blksize);
    fprintf (info, "data_type = %s\n", encode_data_format(hdr->data_type));
    fprintf (info, "hdr_wordorder  = %d (%s)\n", hdr->hdr_wordorder,
	     (hdr->hdr_wordorder == SEED_LITTLE_ENDIAN) ? "intel/vax" :
	     (hdr->hdr_wordorder == SEED_BIG_ENDIAN) ? "sparc/motorola" : 
	     "unknown");
    fprintf (info, "data_wordorder = %d (%s)\n", hdr->data_wordorder,
	     (hdr->data_wordorder == SEED_LITTLE_ENDIAN) ? "intel/vax" :
	     (hdr->data_wordorder == SEED_BIG_ENDIAN) ? "sparc/motorola" : 
	    "unknown");
    steim_comp = ((bs = find_blockette(hdr,1000)) &&
	(bh = (BLOCKETTE_HDR *)(bs->pb)) &&
	! (IS_STEIM_COMP(((BLOCKETTE_1000 *)bh)->format))) ? 0 : 1;
    if (steim_comp) {
	fprintf (info, "num_data_frames = %d\n", hdr->num_data_frames);
	fprintf (info, "x0 = %d\n", hdr->x0);
	fprintf (info, "xn = %d\n", hdr->xn);
    }
    if (hdr_type == DRM_HDR_TYPE) {
    fprintf (info, "Quanterra clock_corr = %d\n", hdr->xm1);
    fprintf (info, "Quanterra SOH = %X\n", hdr->xm2);
    }
    /* dump out blockettes */
    if (hdr->pblockettes != NULL) {
	int next;
	int l;
	int type;
	char *pb;
	BS *bs = hdr->pblockettes;
	do {
	    char *p, str[MAXODHSTRLEN+1];
	    int i;
	    BLOCKETTE_HDR *bh;
	    bh = (BLOCKETTE_HDR *)(bs->pb);
	    pb = NULL;
	    /* If blockette is in the wrong word order, create copy	*/
	    /* and swap wordorder of the copy.				*/
	    if (bs->wordorder != my_wordorder) {
		pb = (char *)malloc(bs->len);
		memcpy (pb, bs->pb, bs->len);
		swab_blockette (bs->type, pb, bs->len);
		bh = (BLOCKETTE_HDR *)(pb);
	    }
	    switch (type=bs->type) {
	      case 100:	
		fprintf (info, "blockette %d, actual_rate = %.6f, flags = 0x%c\n",
			 type,
			 ((BLOCKETTE_100 *)bh)->actual_rate, 
			 ((BLOCKETTE_100 *)bh)->flags);
		break;
	      case 200:	
	      case 201:	
	      case 300:	
	      case 310:	
	      case 320:	
	      case 390:	
	      case 400:	
	      case 405:	
	      case 500:	
		fprintf (info, "blockette %d\n",type);
		break;
	      case 1000: 
		fprintf (info, "blockette %d, fmt = %d (%s), order = %d (%s), len = %d (%d)\n",
			 type, 
			 ((BLOCKETTE_1000 *)bh)->format, 
			 b1000_format_str(((BLOCKETTE_1000 *)bh)->format),
			 ((BLOCKETTE_1000 *)bh)->word_order, 
			 (((BLOCKETTE_1000 *)bh)->word_order == SEED_BIG_ENDIAN) ? "BIG_ENDIAN" :
			 (((BLOCKETTE_1000 *)bh)->word_order == SEED_LITTLE_ENDIAN) ? "LITTLE_ENDIAN" : 
			 "unknown",
			 ((BLOCKETTE_1000 *)bh)->data_rec_len, 
			 roundoff ( pow(2.,(double)((BLOCKETTE_1000 *)bh)->data_rec_len) ) );
		break;
	      case 1001: 
		fprintf (info, "blockette %d, clock_quality = %d, usec99 = %d, flags = 0x%02x (%s), frame_count=%d\n",
			 type, 
			 ((BLOCKETTE_1001 *)bh)->clock_quality, 
			 ((BLOCKETTE_1001 *)bh)->usec99, 
			 ((BLOCKETTE_1001 *)bh)->flags,
			 (((BLOCKETTE_1001 *)bh)->flags & MSHEAR_TIMETAG_FLAG) ? "MSHEAR" : "",
			 ((BLOCKETTE_1001 *)bh)->frame_count );

		break;
	      case 2000:
		fprintf (info, "blockette 2000 (opaque data), len=%d, datalen=%d, "
			 "recno=%d, order=%d (%s), flags=0x%02x, hdr=",
			 ((BLOCKETTE_2000 *)bh)->blockette_len,
			 ((BLOCKETTE_2000 *)bh)->blockette_len - ((BLOCKETTE_2000 *)bh)->data_offset,
			 ((BLOCKETTE_2000 *)bh)->record_num,
			 ((BLOCKETTE_2000 *)bh)->word_order,
			 (((BLOCKETTE_2000 *)bh)->word_order == SEED_BIG_ENDIAN) ? "BIG_ENDIAN" :
			 (((BLOCKETTE_2000 *)bh)->word_order == SEED_LITTLE_ENDIAN) ? "LITTLE_ENDIAN" :
			 "unknown",
 			 ((BLOCKETTE_2000 *)bh)->data_flags);
		p = (char *)bh;
		i = (char *)&(((BLOCKETTE_2000 *)bh)->num_hdr_strings) - p;
		p += i + sizeof(((BLOCKETTE_2000 *)bh)->num_hdr_strings);
		for (i=0; i<((BLOCKETTE_2000 *)bh)->num_hdr_strings; i++) {
		    charvncpy(str,p,MAXODHSTRLEN,i);
		    fprintf (info, "%s%s", ((i>0) ? "," : ""), str);
		}
		fprintf (info, "\n");
		break;
	      default:
		fprintf (info,   "unknown blockette %d, skipping\n", bh->type);
		fprintf (stderr, "unknown blockette %d, skipping\n", bh->type);
	    }
	    if (pb) free (pb);
	    bs = bs->next;
	} while (bs != (BS *)NULL);
    }
    END_BLOCKETTES:
    fprintf (info, "\n");
}

/************************************************************************/
/*  show_stream:							*/
/*	Show info about the stream itself (eg the whole file).		*/
/************************************************************************/
void show_stream 
   (ST_INFO	*st_p)		/* ptr to stream structure.		*/
{
    fprintf (info, "filename =   %s\n", st_p->filename);
    fprintf (info, "station =    %s\n", st_p->station);
    fprintf (info, "channel =    %s\n", st_p->channel);
    fprintf (info, "network =    %s\n", st_p->network);
    fprintf (info, "num_blocks = %d\n", st_p->num_blocks);
    fprintf (info, "blksize =    %d\n", st_p->blksize);
    fprintf (info, "hdr_type =   %d\n", st_p->hdr_type);
    if (st_p->first && st_p->last) {
    fprintf (info, "begtime =    %s\n", 
	     date_string(st_p->first->hdr->begtime, NULL_STR));
    fprintf (info, "endtime =    %s\n", 
	     date_string(st_p->last->hdr->endtime, NULL_STR));
    }
    fprintf (info, "\n");
}


/************************************************************************/
/*  bool_int:								*/
/*	Perform specified boolean operation.				*/
/************************************************************************/
int bool_int
   (OP		op,		/* boolean operator.			*/
    int		v1,		/* first operand.			*/
    int		v2)		/* second operand.			*/
{
    switch (op) {
	case AND:	    return (v1 & v2);
	case OR:	    return (v1 | v2);
	case CLEAR:	    return (v1 | ~v2);
	default:
	    fprintf (info, "Unknown boolean cmd: %d\n", op);
	    return(v1);
    }
}

/************************************************************************/
/*  apply_time_correction:						*/
/*	Apply the time correction to the hdrtime field if it has not	*/
/*	already been applied.  Based on on the flag, either		*/
/*	a)  clear the time correction field and time_correction_applied	*/
/*	bit in the activity flags, OR					*/
/*	b)  set the time_correction_applied bit and leave the time	*/
/*	correction value in the time_correction_field.			*/
/************************************************************************/
void apply_time_correction
   (DATA_HDR	*hdr,		/* ptr to data hdr for record.		*/
    TARGET	flag)		/* flag to keep or clear corr field.	*/
{
    if (hdr->activity_flags&ACTIVITY_TIME_CORR_APPLIED) {
	/*  Correction has already been applied.  No action needed.	*/
    }
    else if (hdr->num_ticks_correction != 0) {
	/*  Apply time correction, and set flag.			*/
	hdr->hdrtime = add_dtime(hdr->hdrtime, 
				 (double)hdr->num_ticks_correction * USECS_PER_TICK);
	hdr->activity_flags |= ACTIVITY_TIME_CORR_APPLIED;
    }
    update_times (hdr);
    switch (flag) {
	case KEEP_FIELD:    break;
	case CLEAR_FIELD: 
	    hdr->num_ticks_correction = 0; 
	    hdr->activity_flags &= ~ ACTIVITY_TIME_CORR_APPLIED; 
	    break;
	default:
	    fprintf (info, "Invalid flag to apply time correction\n");
    }
}

/************************************************************************/
/*  update_times:							*/
/*	Update the begtime and optionally the hdrtime after a time	*/
/*	correction has been made.					*/
/************************************************************************/
void update_times 
   (DATA_HDR	*hdr)		/* ptr to data hdr for record.		*/
{
    int		seconds, usecs;
    /*	Ensure that all times in the header are consistent with header	*/
    /*	info.  If the ACTIVITY_TIME_CORR_APPLIED is NOT set,		*/
    /*	time correction field has NOT been added to the hdrtime, 	*/
    /*	so we must add it to get begtime.				*/
    if (hdr->activity_flags&ACTIVITY_TIME_CORR_APPLIED)
	hdr->begtime = hdr->hdrtime;
    else hdr->begtime =  add_dtime(hdr->hdrtime,
				  (double)hdr->num_ticks_correction * USECS_PER_TICK);
    time_interval2(hdr->num_samples-1,hdr->sample_rate,hdr->sample_rate_mult,
		   &seconds,&usecs);
    hdr->endtime = add_time(hdr->begtime,seconds,usecs);
}

/************************************************************************/
/*  are_contiguous:							*/
/*	Determine if 2 blocks are assumed to be "contiguous" in time.	*/
/*	If so, return TRUE. If not, return FALSE.			*/
/************************************************************************/
int are_contiguous
   (DATA_HDR	*hdr1,		/* ptr to data hdr for first record.	*/
    DATA_HDR	*hdr2)		/* ptr to data hdr for second record.	*/
{
    double	delta;
    int		seconds, usecs;
    INT_TIME	exptime;

    /*	Compute difference between the expected time of hdr2 and begin	*/
    /*	time of hdr2.							*/
    time_interval2 (1, hdr1->sample_rate, hdr1->sample_rate_mult, &seconds, &usecs);
    exptime = add_time (hdr1->endtime, seconds, usecs);
    delta = fabs (tdiff (hdr2->begtime, exptime));
    return ( (delta < (double)TOL(hdr1->sample_rate,hdr1->sample_rate_mult)*USECS_PER_TICK)
	    ? TRUE : FALSE );
}

void set_corr_limit
   (int		rate,		/* sample rate for data.		*/
   int		rate_mult)	/* sample rate_mult for data.		*/
{
    double sps = sps_rate (rate, rate_mult);
    if (sps <= 100) corr_limit = 10;
    if (sps == 80) corr_limit = 5;
    if (sps > 100) corr_limit = 1;
}
