/************************************************************************/
/*  qedit -	Edit quanterra and MiniSEED data files.			*/
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
static char sccsid[] = "$Id: qedit.c,v 1.8 2003/07/12 00:26:14 doug Exp $ ";
#endif

#include <stdio.h>
#include "version.h"

char *syntax[] = {
"%s version " VERSION,
"	[-f date] [-t date | -s interval] [-h] [-D j | m] [-d n]",
"	input_file [ output_file ]",
"    where:",
"	-f date	    From date - ignore data before this date.",
"	-t date	    To date - ignore data from this date on.",
"		    Date can be in formats:",
"			[19]yy/mm/dd/hh:mm:ss.ffff",
"			[19]yy/mm/dd.hh:mm:ss.ffff",
"			[19]yy/mm/dd,hh:mm:ss.ffff",
"			[19]yy.ddd.hh:mm:ss.ffff",
"			[19]yy.ddd,hh:mm:ss.ffff",
"			[19]yy,ddd,hh:mm:ss.ffff",
"	-s interval span interval.  Alternate way of specifying end time.",
"		    Interval can be an integer followed immediately by",
"		    s, m, h, or d for seconds, minutes, hours, or days.",
"	-d n	    Debug output (OR of any of the following values:)",
"			1 = debug cmd",
"			2 = debug execute()",
"			4 = block do()",
"			8 = block detail",
"	-D j | m    Display dates in julian or month and day format.",
"	-h	    Help - prints syntax message.",
"	input_file  specifies the input file to edit.",
"	output_file specifies the output file to be created with edited data.",
"		    If no output file is specified, the input file will be rewritten.",
NULL };

#define	CMDLEN		160
#define	EXTERN

#include "qlib2.h"

#include "params.h"
#include "defs.h"
#include "st_info.h"
#include "externals.h"
#include "procs.h"

#ifdef	USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
char *rl_gets (char *prompt);
#endif

/************************************************************************/
/*  main:   main program.						*/
/************************************************************************/
main (int argc, char **argv)
{
    ST_INFO	*st_p;
    INT_TIME	*stime;
    INT_TIME	*etime;
#ifdef	USE_READLINE
    char	*str;
#else
    char	str[CMDLEN+2];
#endif
    CMD		*cmd;
    FILE	*cmdin = stdin;
    int		n;
    char	*prompt;

    interactive = isatty(fileno(stdin));
    debug_option = 0;
    auto_display = ON;
    info = stdout;
    extended_bounds = 0;
    
    parse_cmdline (&argc, &argv);
    if (interactive) {
	prompt = (char *)malloc (strlen(cmdname)+3);
	sprintf (prompt, "%s> ", cmdname);
    }
    else prompt = "";
    
    /*	Determine the block size and format of each input stream.	*/
    for (st_p = st_head_in.next; st_p != NULL; st_p = st_p->next) {
	if (get_stream_info (st_p) == EOF)
	    st_p = close_stream(st_p);
    }

    /*	Get the first block for each stream.				*/
    for (st_p = st_head_in.next; st_p != NULL; st_p = st_p->next) {
	get_block_info (st_p);
    }

    /*	Read in and save all of the data for each data stream.		*/
    for (st_p = st_head_in.next; st_p != NULL; st_p = st_p->next) {
	stime = (start_flag) ? &start_time : NULL;
	etime = (end_flag) ? &end_time : NULL;
	while ( position_stream (st_p, stime, etime) != EOF ) {
	    store_block (st_p);
	    stime = NULL;
	}
    }

    st_p = st_head_in.next;
    if (st_p) {
	st_p->cur = st_p->first;
	st_p->prevpos = st_p->first;
    }
    while (
#ifdef	USE_READLINE
	   (str=rl_gets (prompt)) != NULL
#else
	   output_prompt (cmdname, prompt), 
	   fgets(str, CMDLEN, cmdin) != NULL
#endif
	   ) 
    {
#ifndef	USE_READLINE
	str[CMDLEN+1] = '\0';
#endif
	n = strlen(str);
	if (str[n-1]=='\n') str[n-1] = '\0';
	if (strlen(str) == 0) continue;
	if ((cmd=parse_cmd(str))==NULL) continue;
	if (debug(DEBUG_CMD)) dump_cmd(cmd);
	if (! resolve_bounds(st_p, cmd)) continue;
	execute (st_p, cmd);
	cmd_free(cmd);
    }
    /* Should never get here.	*/
    return (0);
}

/************************************************************************/
/*  dump_cmd:								*/
/*	Debugging routine to dump command structure.			*/
/************************************************************************/
void dump_cmd
   (CMD		*cmd)		/* ptr to command structure.		*/
{
    printf ("cmd.from   = %s\n", arg_string(&cmd->from, NULL_STR));
    printf ("cmd.to     = %s\n", arg_string(&cmd->to, NULL_STR));
    printf ("cmd.op     = %s\n", op_string(cmd->op, NULL_STR));
    printf ("cmd.target = %s\n", target_string((DATATYPE)cmd->target, NULL_STR));
    printf ("cmd.arg0   = %s\n", arg_string(&cmd->arg[0], NULL_STR));
    printf ("cmd.arg1   = %s\n", arg_string(&cmd->arg[1], NULL_STR));
    printf ("\n");
}

/************************************************************************/
/*  ouput_prompt:							*/
/*	Output prompt before reading next command if reading from term.	*/
/************************************************************************/
void output_prompt
   (char	*cmdname,	/* program name.			*/
    char 	*prompt)	/* prompt to output (NULL -> no prompt)	*/
{
    if (prompt) printf (prompt);
}


#ifdef	USE_READLINE
/* A static variable for holding the line. */
static char *line_read = (char *)NULL;
     
/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char *rl_gets (char *prompt)
{
    /* If the buffer has already been allocated,    */
    /* return the memory to the free pool.	    */
    if (line_read)
    {
	free (line_read);
	line_read = (char *)NULL;
    }
     
    /* Get a line from the user. */
    line_read = readline (prompt);
     
    /* If the line has any text in it, save it on the history. */
    if (line_read && *line_read && prompt)
	add_history (line_read);
     
    return (line_read);
}
#endif
