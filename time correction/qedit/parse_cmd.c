/************************************************************************/
/*  Routines to parse commands.						*/
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
static char sccsid[] = "$Id: parse_cmd.c,v 1.5 2005/01/09 05:56:36 doug Exp $ ";
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

#define	WHITESPACE		" \t\n"
#define	TIME_FMT		0

#define	CMD_ERROR(cmd) \
{       fprintf (info, "invalid command\n"); cmd_free(cmd); return(NULL); }

/************************************************************************/
/*  parse_cmd:								*/
/*	Parse qedit command into command structure.			*/
/************************************************************************/
CMD *parse_cmd
   (char	*str)		/* command string to parse.		*/
{
    char	*token;
    int		i, n;
    CMD		*cmd;

    cmd = (CMD *)malloc(sizeof(CMD));
    cmd->from.argt = cmd->to.argt = NO_TYPE;

    /* Parse optional "from <time> to <time>" prefix.			*/
    token = strtok(str,WHITESPACE);
    if (MATCH_STR(bound_keywd_table[0].string,token)) {
	token = strtok(NULL,WHITESPACE);
	if (!set_bound(&cmd->from,token,bound_keyval_table)) CMD_ERROR(cmd);
	token = strtok(NULL,WHITESPACE);
	if (MATCH_STR(bound_keywd_table[1].string,token)) {
	    cmd->ub = bound_keywd_table[1].key;
	    token = strtok(NULL,WHITESPACE);
	    if (!set_bound(&cmd->to,token,bound_keyval_table)) CMD_ERROR(cmd);
	    token = strtok(NULL,WHITESPACE);
	}
	else if (MATCH_STR(bound_keywd_table[2].string,token)) {
	    cmd->ub = bound_keywd_table[2].key;
	    token = strtok(NULL,WHITESPACE);
	    if (!set_bound(&cmd->to,token,bound_keyval_table)) CMD_ERROR(cmd);
	    token = strtok(NULL,WHITESPACE);
	}
	else CMD_ERROR(cmd);
	if (!typecheck_bounds(cmd->from, cmd->to)) CMD_ERROR(cmd);
    }

    /*	Parse operation and target.	*/
    cmd->op = op_value(token);
    token = strtok(NULL,WHITESPACE);
    cmd->target = target_value(token);
    if (! cmdline_value(cmd)) {
	fprintf (stderr, "** Error - invalid command line\n");
	CMD_ERROR(cmd);
    }


    /*	Parse arguments.		*/
    if (cmd->target) token = strtok(NULL,WHITESPACE);
    for (i=0; i<2; i++) {
	if (cmd->arg[i].argt != NO_TYPE) {
	    if ((n=set_arg (&cmd->arg[i], token))<0) CMD_ERROR(cmd)
	    if (n>0) token = strtok(NULL,WHITESPACE);
	}
    }

    return (cmd);
}

/************************************************************************/
/*  op_value:								*/
/*	Return the command value for the specified string.		*/
/************************************************************************/
OP op_value
   (char    *str)		/* command string to get OP value for.	*/
{
    OP_TABLE	*pt = op_table;
    while (pt->string != NULL && !MATCH_STR(str,pt->string))
	++pt;
    return (pt->op);
}

/************************************************************************/
/*  op_string:								*/
/*	Return the string for the specified command value.		*/
/************************************************************************/
char *op_string
   (OP	    op,			/* command value.			*/
    char    *dflt_str)		/* string for default value.		*/
{
    OP_TABLE	*pt = op_table;
    while (pt->string != NULL && !MATCH_VAL(op,pt->op))
	++pt;
    return ((pt->string!=NULL)?pt->string : dflt_str);
}

/************************************************************************/
/*  target_value:							*/
/*	Return the target value for the specified string.		*/
/************************************************************************/
TARGET target_value
   (char	*str)		/* target string.			*/
{
    TARGET_TABLE	*pt = target_table;
    while (pt->string != NULL && !MATCH_STR(str,pt->string))
	++pt;
    return (pt->target);
}

/************************************************************************/
/*  target_string:							*/
/*	Return the string for the specified target value.		*/
/************************************************************************/
char *target_string
   (DATATYPE	target,		/* target value.			*/
    char    *dflt_str)		/* string for default value.		*/
{
    TARGET_TABLE	*pt = target_table;
    while (pt->target != (int)NULL && !MATCH_VAL(target,pt->target))
	++pt;
    return ((pt->string!=NULL)?pt->string : dflt_str);
}

/************************************************************************/
/*  direction_value:							*/
/*	Return the direction value for the specified string.		*/
/************************************************************************/
DIRECTION direction_value
   (char	*str)		/* direction string.			*/
{
    DIRECTION_TABLE	*pt = direction_table;
    while (pt->string != NULL && !MATCH_STR(str,pt->string))
	++pt;
    return (pt->direction);
}

/************************************************************************/
/*  direction_string:							*/
/*	Return the string for the specified direction value.		*/
/************************************************************************/
char *direction_string
   (DIRECTION direction,	/* direction value.			*/
    char    *dflt_str)		/* string for default value.		*/
{
    DIRECTION_TABLE	*pt = direction_table;
    while (pt->direction != (int)NULL && !MATCH_VAL(direction,pt->direction))
	++pt;
    return ((pt->string!=NULL)?pt->string : dflt_str);
}

/************************************************************************/
/*  datatype_string:							*/
/*	Return the string for the specified datatype value.		*/
/************************************************************************/
char *datatype_string
   (DATATYPE	datatype,	/* datatype value.			*/
    char	*dflt_str)	/* string for default value.		*/
{
    DATATYPE_TABLE	*pt = datatype_table;
    while (pt->datatype != (int)NULL && !MATCH_VAL(datatype,pt->datatype))
	++pt;
    return ((pt->string!=NULL)?pt->string : dflt_str);
}

/************************************************************************/
/*  date_value:								*/
/*	Return the date value for the specified string.			*/
/************************************************************************/
INT_TIME date_value
   (char	*str)		/* date string.				*/
{
    INT_TIME		*ip;
    INT_TIME		time = {0,0,0};
    if (str == NULL) return (time);
    if ((ip=parse_date(str))==NULL) return(time);
    time = *ip;
    return(time);
}

/************************************************************************/
/*  date_string:							*/
/*	Return the string for a specified date.				*/
/************************************************************************/
char *date_string
   (INT_TIME	time,		/* date/time to convert.		*/
    char	*dflt_str)	/* default string value.		*/
{
    char    *p;
    if (missing_time(time)) return (dflt_str);
    p = time_to_str(time,TIME_FMT);
    return ((p!=NULL) ? p : dflt_str);
}

/************************************************************************/
/*  udate_string:							*/
/*	Return the string for a specified date.				*/
/************************************************************************/
char *udate_string
   (INT_TIME	time,		/* date/time to convert.		*/
    char	*dflt_str)	/* default string value.		*/
{
    char    *p;
    if (missing_time(time)) return (dflt_str);
    p = utime_to_str(time,TIME_FMT);
    return ((p!=NULL) ? p : dflt_str);
}

/************************************************************************/
/*  interval_string:							*/
/*	Return the string for a specified inverval.			*/
/************************************************************************/
char *interval_string
   (EXT_TIME	*interval,	/* interval to convert.			*/
    char	*dflt_str)	/* default string.			*/
{
    char	*p;
    if (interval == NULL) return (NULL);
    p = interval_to_str(*interval,TIME_FMT);
    return ((p!=NULL) ? p : dflt_str);
}

/************************************************************************/
/*  cmdline_value:							*/
/*	Return the cmdline value for the specified cmd.			*/
/************************************************************************/
int cmdline_value
   (CMD		*cmd)		/* command string.			*/
{
    CMD_TABLE	*pt = cmd_table;
    while (pt->op != NO_OP && ! (pt->op == cmd->op && pt->target == cmd->target))
	++pt;
    cmd->arg[0].argt = pt->arg_0;
    cmd->arg[1].argt = pt->arg_1;
    return (pt->op != NO_OP);
}


/************************************************************************/
/*  set_arg:								*/
/*	Set the argument value for the specified argument.		*/
/*  returns:								*/
/*	-1 = error - invalid argument.					*/
/*	 0 = no optional argument.					*/
/*	 1 = valid argument.						*/
/************************************************************************/
int set_arg 
   (ARG		*arg,		/* ptr to argument structure.		*/
    char	*token)		/* argument string.			*/
{
    int		nval;
    char	*sp;
    INT_TIME	*ip, *dp;
    EXT_TIME	*ep;

    switch (arg->argt) {
	case OPT_INT_TYPE:
	    if (token == NULL) { arg->argt = NO_TYPE; arg-> argv.str = NULL; return(0); }
	case INT_TYPE:	
	    arg->argt = INT_TYPE;
	    if (token == NULL) return(-1);
	    nval = strtol (token, &sp, 10);
	    if (sp!= token+strlen(token)) return(-1);
	    arg->argv.ival = nval;
	    break;
	case OPT_HEX_TYPE:
	    if (token == NULL) { arg->argt = NO_TYPE; arg-> argv.str = NULL; return(0); }
	case HEX_TYPE:	
	    arg->argt = HEX_TYPE;
	    if (token == NULL) return(-1);
	    nval = strtol (token, &sp, 16);
	    if (sp!= token+strlen(token)) return(-1);
	    arg->argv.hval = nval;
	    break;
	case OPT_STR_TYPE:
	    if (token == NULL) { arg->argt = NO_TYPE; arg-> argv.str = NULL; return(0); }
	case STR_TYPE:	
	    arg->argt = STR_TYPE;
	    if (token == NULL) return(-1);
	    arg->argv.str = (char *) malloc(strlen(token) + 1);
	    strcpy (arg->argv.str, token);
	    break;
	case OPT_INTERVAL_TYPE:
	    if (token == NULL) { arg->argt = NO_TYPE; arg-> argv.str = NULL; return(0); }
	case INTERVAL_TYPE:
	    arg->argt = INTERVAL_TYPE;
	    /*	This is assumed to be a time interval, NOT a date.  */
	    /*	The day field begins with 0, not 1.		    */
	    /*	No year or month can be specified.		    */
	    if (token == NULL) return(-1);
	    if ((ep=parse_interval(token))==NULL) return(-1);
	    arg->argv.interval = (EXT_TIME *) malloc(sizeof(EXT_TIME));
	    *arg->argv.interval = *ep;
	    break;
	case OPT_DATE_TYPE:
	    if (token == NULL) { arg->argt = NO_TYPE; arg-> argv.str = NULL; return(0); }
	case DATE_TYPE:
	    arg->argt = DATE_TYPE;
	    /*	This is assumed to be a date, NOT a time interval.  */
	    if (token == NULL) return(-1);
	    if ((ip=parse_date(token))==NULL) return(-1);
	    arg->argv.time = (INT_TIME *) malloc(sizeof(INT_TIME));
	    *arg->argv.time = *ip;
	    break;
	case NO_TYPE:
	    arg->argt = STR_TYPE;
	    /*	Should never get here.	*/
	    fprintf (stderr, "arg type error\n");
	    exit(1);
	    break;
	default:
	    /*  Should never get here. */
	    fprintf (stderr, "arg type error\n");
	    exit(1);
	    break;
    }
    return(1);
}
    

/************************************************************************/
/*  arg_str:								*/
/*	Return the string for a specified argument.			*/
/************************************************************************/
char *arg_string
   (ARG		*arg,		/* ptr to argument structure.		*/
    char	*dflt_str)	/* default string.			*/
{
    char *p;
    static char  str[80];
    if (arg == NULL) return (NULL);
    switch (arg->argt) {
	case NO_TYPE:	return(dflt_str); break;
	case INT_TYPE:
	    sprintf (str, "%d", arg->argv.hval);
	    return((char *)str);
	case HEX_TYPE:
	    sprintf (str, "0x%x", arg->argv.hval);
	    return((char *)str);
	case STR_TYPE:
	    return((arg->argv.str!=NULL) ? arg->argv.str : dflt_str);
	case INTERVAL_TYPE:
	    p = interval_string(arg->argv.interval, NULL_STR);
	    return ((p!=NULL) ? p : dflt_str);
	case DATE_TYPE:
	    p = date_string(*arg->argv.time, NULL_STR);
	    return ((p!=NULL) ? p : dflt_str);
	default:
	    /*  Should never get here.	*/
	    fprintf (stderr, "invalid arg type: %d\n", arg->argt);
	    exit(1);
    }
}

#define	    ARG_FREE(arg) \
    if (arg.argt==STR_TYPE && arg.argv.str!=NULL) free (arg.argv.str); \
    if (arg.argt==DATE_TYPE && arg.argv.time!=NULL) free (arg.argv.time); \
    if (arg.argt==INTERVAL_TYPE && arg.argv.interval!=NULL) free (arg.argv.interval);

/************************************************************************/
/*  cmd_free:								*/
/*	Free all space allocated for a cmd structure.			*/
/************************************************************************/
void cmd_free
   (CMD		*cmd)		/* ptr to command structure.		*/
{   ARG_FREE(cmd->from);
    ARG_FREE(cmd->to);
    ARG_FREE(cmd->arg[0]);
    ARG_FREE(cmd->arg[1]);
    free (cmd);
}

