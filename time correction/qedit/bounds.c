/************************************************************************/
/*  Routines for bounds processing.					*/
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
static char sccsid[] = "$Id: bounds.c,v 1.6 2003/07/12 00:26:13 doug Exp $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

/************************************************************************/
/*  set_bound:								*/
/*	Set the boundary value for a boundary range.			*/
/************************************************************************/
int set_bound
   (ARG		*arg,		/* ptr to ARG structure.		*/
    char	*token,		/* token string.			*/
    BOUND_KEYVAL_TABLE *pt)	/* ptr to boundary keyval table.	*/
{
    /*	Try for integer value, time value, or string value.		*/
    arg->argt = INT_TYPE;
    if (set_arg (arg, token) > 0) return (TRUE);
    arg->argt = DATE_TYPE;
    if (set_arg (arg, token) > 0) return (TRUE);
    arg->argt = STR_TYPE;
    while (pt->string != NULL && !MATCH_STR(token,pt->string))
	    ++pt;
    if (pt->string != NULL && (set_arg(arg, token) > 0)) return (TRUE);
    arg->argt = NO_TYPE;
    return (FALSE);
}

/************************************************************************/
/*  bound_str_value:							*/
/*	Return the command value for the specified string.		*/
/************************************************************************/
BOUND_KEYVAL bound_str_value
   (char	*str)		/* string version of bound.		*/
{
    BOUND_KEYVAL_TABLE	*pt = bound_keyval_table;
    while (pt->string != NULL && !MATCH_STR(str,pt->string))
	++pt;
    return (pt->key);
}

/************************************************************************/
/*  typecheck_bounds:							*/
/*	Typecheck the boundary values.					*/
/************************************************************************/
int typecheck_bounds
   (ARG		from,		/* start bound.				*/
    ARG		to)		/* end bound.				*/
{
    BOUND_KEYVAL    res;
    if (from.argt == STR_TYPE && bound_str_value(from.argv.str) == NO_KEYVAL) 
	return(FALSE);
    if (to.argt == STR_TYPE && bound_str_value(to.argv.str) == NO_KEYVAL) 
	return(FALSE);
    return(TRUE);
}

/************************************************************************/
/*  cvt_str_bound:							*/
/*	Convert string bound to valid boundary value.			*/
/************************************************************************/
int cvt_str_bound
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    ARG		*arg,		/* ptr to ARG structure.		*/
    int		type)		/* type of bound specified.		*/
{
    BOUND_KEYVAL    b_val = bound_str_value(arg->argv.str);
    char	    *p = arg->argv.str;
    switch (b_val) {
	case FIRST_RECORD:
	    switch (type) {
		case INT_TYPE:  arg->argv.ival = st_p->first->blk_no; break;
		case DATE_TYPE: 
		    if ((arg->argv.time=(INT_TIME *)malloc(sizeof(INT_TIME)))==NULL) {
			fprintf (info, "unable to malloc date\n"); exit(1); }
		    *arg->argv.time = st_p->first->hdr->begtime;
		    break;
		default:
		    fprintf (info, "invalid boundary type: %d\n", type);
		    return(FALSE);
	    }
	    break;
	case PREVIOUS_RECORD: 
	    switch (type) {
		case INT_TYPE:  arg->argv.ival = st_p->prevpos->blk_no; break;
		case DATE_TYPE: 
		    if ((arg->argv.time=(INT_TIME *)malloc(sizeof(INT_TIME)))==NULL) {
			fprintf (info, "unable to malloc date\n"); exit(1); }
		    *arg->argv.time = st_p->prevpos->hdr->begtime;
		    break;
		default:
		    fprintf (info, "invalid boundary type: %d\n", type);
		    return(FALSE);
	    }
	    break;
	case CURRENT_RECORD: 
	    switch (type) {
		case INT_TYPE:  arg->argv.ival = st_p->cur->blk_no; break;
		case DATE_TYPE: 
		    if ((arg->argv.time=(INT_TIME *)malloc(sizeof(INT_TIME)))==NULL) {
			fprintf (info, "unable to malloc date\n"); exit(1); }
		    *arg->argv.time = st_p->cur->hdr->begtime;
		    break;
		default:
		    fprintf (info, "invalid boundary type: %d\n", type);
		    return(FALSE);
	    }
	    break;
	case LAST_RECORD:
	    switch (type) {
		case INT_TYPE:  arg->argv.ival = st_p->last->blk_no; break;
		case DATE_TYPE: 
		    if ((arg->argv.time=(INT_TIME *)malloc(sizeof(INT_TIME)))==NULL) {
			fprintf (info, "unable to malloc date\n"); exit(1); }
		    *arg->argv.time = st_p->last->hdr->begtime;
		    break;
		default:
		    fprintf (info, "invalid boundary type: %d\n", type);
		    return(FALSE);
	    }
	    break;
    }
    arg->argt = type;
    free(p);
    return (TRUE);
}

/************************************************************************/
/*  resolve_bounds:							*/
/*	Resolve the bound values.					*/
/************************************************************************/
int resolve_bounds
   (ST_INFO	*st_p,		/* ptr to stream structure.		*/
    CMD		*cmd)		/* command to resolve bound for.	*/
{
    int result = TRUE;
    /*	Resolve boundaries.  Replace string values with block numbers.	*/

    if (cmd->from.argt == NO_TYPE && cmd->to.argt == NO_TYPE) return (TRUE);
    if (st_p == NULL || st_p->first == NULL || st_p->last == NULL) {
	if (interactive) fprintf (info, "unable to resolve bounds on empty file\n");
	return (FALSE);
    }
    if (st_p == NULL) return (FALSE);
    if (cmd->from.argt == STR_TYPE && result) {
	result = cvt_str_bound(st_p,&cmd->from,INT_TYPE);
	if (result == FALSE) fprintf (info, "unable to convert bound \"s\" to block number\n",
				      cmd->from.argv.str);
    }
    if (cmd->to.argt == STR_TYPE && result) {
	result = cvt_str_bound(st_p,&cmd->to,INT_TYPE);
	if (result == FALSE) fprintf (info, "unable to convert bound \"s\" to block number\n",
				      cmd->from.argv.str);
    }
    return(result);
}
