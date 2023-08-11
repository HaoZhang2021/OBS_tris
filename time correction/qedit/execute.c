/************************************************************************/
/*  Routines to execute command.					*/
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
static char sccsid[] = "$Id: execute.c,v 1.10 2005/03/21 20:03:57 doug Exp $ ";
#endif

#include <stdio.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

#define UNKNOWN_DIRECTION(direction) { \
    fprintf (info, "Unknown direction: %d\n", (int)direction); \
    ++no_auto_display; }

static INT_TIME	*from_t;
static INT_TIME	*to_t;
static INT_TIME to_time, from_time;

/************************************************************************/
/*  execute:								*/
/*	Execute the command, complete with iteration.			*/
/*  return:								*/
/*	TRUE on success, FALSE on failure.				*/
/************************************************************************/
int execute
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* ptr to cmd to execute.		*/
{
    ARG		save_from, save_to;
    int		status;

    if (cmd == NULL) return (TRUE);
    from_t = to_t = NULL;

    /*	If there is a from...to interval, we have to find the times	*/
    /*	before we start, since the ADD_TIME command needs to	   	*/
    /*	compute its value based on the end time intervals.	    	*/

    /********************************************************************/
    /*				NOTE WELL			    	
    								    	
    	We now have inclusive and no-inclusive upper bounds.	    	
    	Sequence Number:					    	
	a.  If the upper bound was specified with the keyword THRU,
	it represents a CLOSED or INCLUSIVE interval.
	b.  If the upper bound was specified with the keyword TO,
	it represents a OPEN or NON-INCLUSIVE interval.

    	This allows you to use a syntax like:			    	
    	    from 1 thru 100, or from first to last			
    	and you can manipulate that inclusive block interval.		
    								    
    	If the upper bound is a date, it represents an OPEN	    
    	interval, i.e. process up TO, but NOT INCLUDING that date.  
	Currently, this is true for both TO and THRU keywords.
    	This allows you to use a syntax like:			    
    	    from 92.120,00:00 to 92.120,12:00
    	to represent a 12 hour period.
    /****************************************************************/

    /* Check to see if iteration is required.				*/
    switch (cmd->op) {
	case RESEQUENCE:
	case SMOOTH:
	case ADD_TREND:
	case TCORR:
	    if (cmd->from.argt == NO_TYPE) {
		fprintf (info, "Error: this command requires iteration.\n");
		return(FALSE);
	    }
    }

    /* Process the command.						*/
    switch (cmd->op) {

	/* TCORR requires double iteration:
	/*  first backwards from reftime, then forward from reftime.	*/
	case TCORR:  
	    if (cmd->from.argt == NO_TYPE) {
		fprintf (info, "Iteration is required for %s cmd\n", 
			 op_string(TCORR, NULL_STR));
		return(FALSE);
	    }
	    save_from = cmd->from;
	    save_to = cmd->to;
	    /* Iterate backwards from ref ... from. */
	    cmd->from = save_from;
	    cmd->to = cmd->arg[1];
	    if (!find_block (st_p, &cmd->arg[1])) {
		fprintf (info, "unable to find reference block\n");
		return(FALSE);
	    }
	    status = iterate (st_p, cmd, REVERSE_DIR);
	    if (status != TRUE) break;
	    /* Iterate forwards from ref ... to/thru. */
	    /* Start again with reference block. */
	    if (!find_block (st_p, &cmd->arg[1])) {
		fprintf (info, "unable to find reference block\n");
		return(FALSE);
	    }
	    cmd->from = cmd->arg[1];
	    cmd->to = save_to;
	    status = iterate (st_p, cmd, FORWARD_DIR);
	    break;

	case ADD_TREND:
	    /* The add_trend command needs to have actual times for	*/
	    /* the from and to values, if iteration was specified.	*/
	    /* If times were not specfied, find the specified block and	*/
	    /* get its begtime.						*/
	    if (cmd->to.argt != NO_TYPE) {
		if (cmd->to.argt == DATE_TYPE) {
		    to_time = *cmd->to.argv.time;
		    to_t = &to_time;
		}
		else {
		    if (find_block(st_p, &cmd->to)) {
			to_time = st_p->cur->hdr->begtime;
			to_t = &to_time;
		    }
		    else {
			fprintf (info, "unable to find to block\n");
			return (FALSE);
		    }
		}
	    }
	    if (cmd->from.argt != NO_TYPE) {
		if (cmd->from.argt == DATE_TYPE) {
		    from_time = *cmd->from.argv.time;
		    from_t = &from_time;
		}
		else {
		    if (find_block(st_p, &cmd->from)) {
			from_time = st_p->cur->hdr->begtime;
			from_t = &from_time;
		    }
		    else {
			fprintf (info, "unable to find from block\n");
			return (FALSE);
		    }
		}
	    }

	default:
	    /* If iteration was specified, find the from block.			*/
	    if (cmd->from.argt != NO_TYPE) {
		if (!find_block (st_p, &cmd->from)) {
		    fprintf (info, "unable to find from block\n");
		    return(FALSE);
		}
		if (!within_bound (st_p->cur,&cmd->from,&cmd->to,cmd->ub,FORWARD_DIR)) return (FALSE);
	    }
	    status = iterate (st_p, cmd, FORWARD_DIR);
	    break;
    }
    /*	Determine whether we need to display the current block after	*/
    /*	final iteration.						*/
    if (auto_display == ON && !no_auto_display && st_p && st_p->cur != NULL) 
	show_hdr(st_p->cur, st_p->hdr_type);
    return (status);
}

int iterate
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd,		/* ptr to cmd to execute.		*/
    DIRECTION	direction)
{
    BLKINFO	*next_record;
    int		first_pass = 1;

    if (debug(DEBUG_EXECUTE)) fprintf (info, "iterate %s\n", 
				   direction_string(direction,"UNKNOWN"));

loop:

    if (debug(DEBUG_EXECUTE)) {
	fprintf (info, "loop: from = %s", arg_string(&cmd->from, NULL_STR));
	fprintf (info, "      to   = %s", arg_string(&cmd->to, NULL_STR));
	fprintf (info, "      cur  = ");
	show_hdr_summary((st_p) ? st_p->cur : (BLKINFO *)NULL);
    }

    no_auto_display = 0;
    if (st_p == NULL) switch (cmd->op) {
	case HELP:  
	    do_help(st_p, cmd); break;
	case QUIT:
	    exit(0); break;
	default:
	    if (interactive) fprintf (info, "Command not valid on empty file\n");
	    return(FALSE);
	break;
    }
    else switch (cmd->op) {
	case HELP:
	    do_help(st_p, cmd); break;
	case FIND:	    
	    do_find(st_p, cmd); break;
	case VERIFY:	    
	    do_verify(st_p, cmd); break;
	case BOUNDS:
	    do_bounds(st_p, cmd); break;
	case DELETE:
	    do_delete(st_p, cmd); break;
	case FORWARD:
	case BACK:
	case FIRST:
	case LAST:
	    do_position(st_p, cmd); break;
	case DISPLAY:
	    do_display(st_p, cmd); break;
	case ADD:
	    do_add(st_p, cmd); break;
	case SET:
	    do_set(st_p, cmd); break;
	case AND:
	case OR:
	    do_bool(st_p, cmd); break;
	case ADD_TIME:
	    do_add_time(st_p, cmd); break;
	case SET_TIME:
	    do_set_time(st_p, cmd); break;
	case AUTODISPLAY:
	    do_autodisplay(st_p, cmd); break;
	case APPLY_CORR:
	    do_apply_corr(st_p, cmd); break;
	case QUIT:
	    exit(0); break;
	case WRITE:
	case FILEQUIT:
	    do_write_file(st_p, cmd, outfile); break;
	case ADD_TREND:
	    do_add_trend(st_p, cmd, from_t, to_t); break;
	case RESEQUENCE:
	    do_resequence(st_p, cmd, first_pass); break;
	case LIMIT:
	    do_limit(st_p, cmd); break;
	case SMOOTH:
	    do_smooth(st_p, cmd, direction, first_pass); break;
	case GETPCF:
	    do_getpcf(st_p, cmd); break;
	case TCORR:
	    do_tcorr(st_p, cmd, direction, first_pass); break;
	case VOLHDR:
	    do_volhdr(st_p, cmd); break;
	default:
	    fprintf (info, "Unimplemented command: %s %s\n",
		     op_string(cmd->op, NULL_STR), 
		     target_string((DATATYPE)cmd->target, NULL_STR));
	    break;
    }

    /*	Determine whether we need to display the current block after 	*/
    /*	this iteration.							*/
    if (cmd->to.argt == NO_TYPE) {
/*::
	if ((st_p != NULL) && auto_display != OFF && !no_auto_display && st_p->cur != NULL) 
	    show_hdr(st_p->cur, st_p->hdr_type);
::*/
	return (TRUE);
    }
    if (st_p != NULL && auto_display == ALL && !no_auto_display && st_p->cur != NULL) 
	show_hdr(st_p->cur, st_p->hdr_type);

    /*	Determine whether we have more iterations to go in loop.	*/
    first_pass = 0;
    switch (direction) {
	case FORWARD_DIR:
	    /* Delete already moved us to the next record. */
	    next_record = (cmd->op == DELETE) ? st_p->cur : st_p->cur->next; 
	    break;
	case REVERSE_DIR:
	    next_record = st_p->cur->prev;
	    break;
	default:
	    UNKNOWN_DIRECTION(direction); return(FALSE);
    }
    if (within_bound(next_record,&cmd->from,&cmd->to,cmd->ub, direction)) {
	st_p->cur = next_record;
	goto loop;
    }
    return (TRUE);
}

/************************************************************************/
/*  within_bound:							*/
/*	Determine whether we are still within bounds.			*/
/*  return:								*/
/*	TRUE on success, FALSE on failure.				*/
/************************************************************************/
int within_bound
   (BLKINFO	*bp,		/* ptr to block info.			*/
    ARG		*lower,		/* ptr to lower arg iteration structure.*/
    ARG		*upper,		/* ptr to upper arg iteration structure.*/
    BOUND_KEYWD ub,		/* type of final bound (TO or THRU).	*/
    DIRECTION	direction)	/* direction of movement.		*/
{
    if (bp == NULL) return (FALSE);
    switch (direction) {
	case (FORWARD_DIR):
	    switch (upper->argt) {
		case NO_TYPE:	return(TRUE);
		case INT_TYPE:
		    if (ub == TO)   return (bp->blk_no <  upper->argv.ival);
		    if (ub == THRU) return (bp->blk_no <= upper->argv.ival);
		case DATE_TYPE:
		    /* TO and THRU have the same meaning for time.	*/
		    return ((tdiff(*upper->argv.time,bp->hdr->begtime) > 0) ? TRUE : FALSE);
		default:
		    fprintf (info, "unknown bound type: %d\n", upper->argt);
		    return(FALSE);
	    }
	    break;
	case (REVERSE_DIR):
	    /* Reverse is always THRU */
	    switch (lower->argt) {
		case NO_TYPE:	return(TRUE);
		case INT_TYPE:
		    return (bp->blk_no >= lower->argv.ival);
		case DATE_TYPE:
		    /* TO and THRU have the same meaning for time.	*/
		    return ((tdiff(*lower->argv.time,bp->hdr->endtime) <= 0) ? TRUE : FALSE);
		default:
		    fprintf (info, "unknown bound type: %d\n", lower->argt);
		    return(FALSE);
	    }
	    break;
	default:
	    UNKNOWN_DIRECTION(direction); return(FALSE);
    }
}

/************************************************************************/
/*  find_block:								*/
/*	Find the specified block by number or date.			*/
/*  return:								*/
/*	TRUE on success, FALSE on failure.				*/
/*	On success, st_p->cur is set to the specified block.		*/
/*	On failure, st_p->cur is set to the last block.			*/
/************************************************************************/
int find_block
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    ARG		*arg)		/* ptr to ARG structure.		*/
{
    int		found = FALSE;
    BLKINFO	*bp;

    /*	Start at the first block and walk forward through the blocks.	*/
    st_p->cur = st_p->first;
    for (st_p->cur=st_p->first; st_p->cur!=NULL; st_p->cur=st_p->cur->next) {
	switch (arg->argt) {
	    case INT_TYPE:
		    /*  Look for matching block number. */
		if (st_p->cur->blk_no == arg->argv.ival) ++found;
		break;
	    case DATE_TYPE:
		/*  Look to see if date is within block. */
		/* ?????? */
		if (tdiff(*arg->argv.time,st_p->cur->hdr->begtime) >= 0 &&
		    tdiff(*arg->argv.time,st_p->cur->hdr->endtime) <= 0) ++found;
		if ((!found) && extended_bounds && 
		    tdiff(*arg->argv.time,st_p->cur->hdr->begtime) <= 0) ++found;
		break;
	    default:
		fprintf (info, "unknown bound type: %d\n", arg->argt);
		return(FALSE);
	}
	if (found) return (TRUE);
    }
    st_p->cur = st_p->last;
    return (FALSE);
}
