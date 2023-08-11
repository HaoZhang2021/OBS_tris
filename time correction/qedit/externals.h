/************************************************************************/
/*  External variables.							*/
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

/*	$Id: externals.h,v 1.13 2004/01/21 03:50:23 doug Exp $ 	*/

/************************************************************************/

#ifdef	EXTERN
OP_TABLE op_table[] = {
/*----------------------------------------------------------------------*/
/*	Command String	Command Definition				*/
/*----------------------------------------------------------------------*/
	"auto",		AUTODISPLAY,
	"find",		FIND,
	"first",	FIRST,
	"top",		FIRST,
	"last",		LAST,
	"bot",		LAST,
	"for",		FORWARD,
	"next",		FORWARD,
	"back",		BACK,
	"prev",		BACK,
	"disp",		DISPLAY,
	"add",		ADD,
	"set",		SET,
	"clear",	CLEAR,
	"and",		AND,
	"or",		OR,
	"del",		DELETE,
	"add_time",	ADD_TIME,
	"add_trend",	ADD_TREND,
	"set_time",	SET_TIME,
	"quit",		QUIT,
	"help",		HELP,
	"write",	WRITE,
	"verify",	VERIFY,
	"file",		FILEQUIT,
	"bounds",	BOUNDS,
	"apply_corr",	APPLY_CORR,
	"resequence",	RESEQUENCE,
	"limit",	LIMIT,
	"smooth",	SMOOTH,
	"getpcf",	GETPCF,
	"tcorr",	TCORR,
	"volhdr",	VOLHDR,
	NULL,		NO_OP,
};
#else
extern OP_TABLE op_table[];
#endif

/************************************************************************/

#ifdef	EXTERN
TARGET_TABLE target_table[] = {
/*----------------------------------------------------------------------*/
/*	Target String	Target Definition				*/
/*----------------------------------------------------------------------*/
	"station",	STATION,
	"loc",		LOCATION,
	"chan",		CHANNEL,
	"net",		NETWORK,
	"seq",		SEQNO,
	"tear",		TEAR,
	"nsamples",	NSAMPLES,
/*::
	"rate_factor",	RATE_FACTOR,
::*/
	"rate",		RATE,
	"rate_mult",	RATE_MULT,
	"aflags",	AFLAGS,
	"ioflags",	IOFLAGS,
	"qflags",	QFLAGS,
	"x0",		X0,
	"xn",		XN,
	"rectype",	RECORDTYPE,
	"corr",		CORRECTION,
	"hdrtime",	HDRTIME,
	"begtime",	BEGTIME,
	"time",		TIME,
	"rec",		RECORD,
	"recno",	RECNO,
	"blk",		BLOCK,
	"hdr",		HEADER,
	"str",		STREAM,
	"on",		ON,
	"off",		OFF,
	"all",		ALL,
	"keep",		KEEP_FIELD,
	"clear",	CLEAR_FIELD,
	"pcf", 		PCF,
	"reftime",	REFTIME,
	"mingap",	MINGAP,
	"maxgap",	MAXGAP,
	NULL,		NO_TARGET,
};
#else
extern TARGET_TABLE target_table[];
#endif

/************************************************************************/

#ifdef	EXTERN
CMD_TABLE cmd_table[] = {
/*--------------------------------------------------------------------------------------*/
/*	Command		Target		Arg_1		Arg_2	    Description		*/
/*--------------------------------------------------------------------------------------*/
	QUIT,		NO_TARGET,	NO_TYPE,	NO_TYPE,    "Quit the program.",
	WRITE,		NO_TARGET,	OPT_STR_TYPE,	NO_TYPE,    "Write output file.",
	FILEQUIT,	NO_TARGET,	OPT_STR_TYPE,	NO_TYPE,    "Write output file and quit.",
	HELP,		NO_TARGET,	OPT_STR_TYPE,	NO_TYPE,    "Print this help list.",
	AUTODISPLAY,	OFF,		NO_TYPE,	NO_TYPE,    "Disable auto current header display.",
	AUTODISPLAY,	ON,		NO_TYPE,	NO_TYPE,    "Display current header after each command.",
	AUTODISPLAY,	ALL,		NO_TYPE,	NO_TYPE,    "Display current header after each iteration.",
	BOUNDS,		ON,		NO_TYPE,	NO_TYPE,    "Bounds for iteration must be within the span of the file.",
	BOUNDS,		OFF,		NO_TYPE,	NO_TYPE,    "Bounds for iteration may exceed span of the file.",
	FIRST,		NO_TARGET,	NO_TYPE,	NO_TYPE,    "Position to first record.",
	LAST,		NO_TARGET,	NO_TYPE,	NO_TYPE,    "Position to last record.",
	FIND,		RECORD,		INT_TYPE,	NO_TYPE,    "Find record with specified record number.",
	FIND,		RECNO,		INT_TYPE,	NO_TYPE,    "Synonym for 'find rec' command.",
	FIND,		BLOCK,		INT_TYPE,	NO_TYPE,    "Synonym for 'find rec' command.",
	FIND,		SEQNO,		INT_TYPE,	NO_TYPE,    "Find record with specified sequence number.",
	FIND,		HDRTIME,	DATE_TYPE,	NO_TYPE,    "Find record with specified header time.",
	FIND,		BEGTIME,	DATE_TYPE,	NO_TYPE,    "Find record with specified begin time.",
	FIND,		TIME,		DATE_TYPE,	NO_TYPE,    "Find record containing time.",
	FIND,		TEAR,		INT_TYPE,	NO_TYPE,    "Find a time tear >= <N> ticks.",
	FORWARD,	NO_TARGET,	OPT_INT_TYPE,	NO_TYPE,    "Advance <N> records.",
	BACK,		NO_TARGET,	OPT_INT_TYPE,	NO_TYPE,    "Back up <N> records.",
	DISPLAY,	NO_TYPE,	NO_TYPE,	NO_TYPE,    "Display current header.",
	DISPLAY,	STREAM,		NO_TYPE,	NO_TYPE,    "Display stream info.",
	DELETE,		NO_TARGET,	OPT_INT_TYPE,	NO_TYPE,    "Delete next <N> record.",
	SET,		STATION,	STR_TYPE,	NO_TYPE,    "Set station to <string>.",
	SET,		LOCATION,	STR_TYPE,	NO_TYPE,    "Set location to <string>.",
	SET,		CHANNEL,	STR_TYPE,	NO_TYPE,    "Set channel to <string>.",
	SET,		NETWORK,	STR_TYPE,	NO_TYPE,    "Set network to <string>.",
	SET,		NSAMPLES,	INT_TYPE,	NO_TYPE,    "Set nsamples to <N>.",
/*::
	SET,		RATE_FACTOR,	INT_TYPE,	NO_TYPE,    "Set rate_factor to <N>.",
::*/
	SET,		RATE,		INT_TYPE,	NO_TYPE,    "Set rate_factor to <N>.",
	SET,		RATE_MULT,	INT_TYPE,	NO_TYPE,    "Set rate_multiplier to <N>.",
	SET,		SEQNO,		INT_TYPE,	NO_TYPE,    "Set sequence_number to <N>.",
	SET,		CORRECTION,	INT_TYPE,	NO_TYPE,    "Set time_correction to <N>.",
	SET,		AFLAGS,		HEX_TYPE,	NO_TYPE,    "Set aflags to <0xN>.",
	SET,		IOFLAGS,	HEX_TYPE,	NO_TYPE,    "Set ioflags to <0xN>.",
	SET,		QFLAGS,		HEX_TYPE,	NO_TYPE,    "Set qflags to <0xN>.",
	SET,		X0,		INT_TYPE,	NO_TYPE,    "Set X0 to <N>.",
	SET,		XN,		INT_TYPE,	NO_TYPE,    "Set Xn to <N>.",
	SET,		RECORDTYPE,	STR_TYPE,	NO_TYPE,    "Set record type (data quality) to <string>.",
	VERIFY,		STATION,	OPT_STR_TYPE,	NO_TYPE,    "Verify that station is consistent.",
	VERIFY,		LOCATION,	OPT_STR_TYPE,	NO_TYPE,    "Verify that location is consistent.",
	VERIFY,		CHANNEL,	OPT_STR_TYPE,	NO_TYPE,    "Verify that channel is consistent.",
	VERIFY,		NETWORK,	OPT_STR_TYPE,	NO_TYPE,    "Verify that network is consistent.",
	VERIFY,		RECORDTYPE,	OPT_STR_TYPE,	NO_TYPE,    "Verify that record type is consistent.",
	VERIFY,		ALL,		NO_TYPE,	NO_TYPE,    "Verify all header fields are consistent.",
	ADD,		NSAMPLES,	INT_TYPE,	NO_TYPE,    "Add <N> to nsamples.",
/*::
	ADD,		RATE_FACTOR,	INT_TYPE,	NO_TYPE,    "Add <N> to rate_factor.",
	ADD,		RATE_MULT,	INT_TYPE,	NO_TYPE,    "Add <N> to rate_multiplier.",
::*/
	ADD,		RATE,		INT_TYPE,	NO_TYPE,    "Add <N> to rate_factor.",
	ADD,		RATE_MULT,	INT_TYPE,	NO_TYPE,    "Add <N> to rate_multiplier.",
	ADD,		CORRECTION,	INT_TYPE,	NO_TYPE,    "Add <N> to time_correction.",
	ADD,		X0,		INT_TYPE,	NO_TYPE,    "Add <N> to X0.",
	ADD,		XN,		INT_TYPE,	NO_TYPE,    "Add <N> to Xn.",
	CLEAR,		AFLAGS,		HEX_TYPE,	NO_TYPE,    "BitClear <~0xN> in aflags.",
	CLEAR,		IOFLAGS,	HEX_TYPE,	NO_TYPE,    "BitClear <~0xN> in ioflags.",
	CLEAR,		QFLAGS,		HEX_TYPE,	NO_TYPE,    "BitClear <~0xN> in qflags.",
	AND,		AFLAGS,		HEX_TYPE,	NO_TYPE,    "BitAND <0xN> in aflags.",
	AND,		IOFLAGS,	HEX_TYPE,	NO_TYPE,    "BitAND <0xN> in ioflags.",
	AND,		QFLAGS,		HEX_TYPE,	NO_TYPE,    "BitAND <0xN> in qflags.",
	OR,		AFLAGS,		HEX_TYPE,	NO_TYPE,    "BitOR <0xN> in aflags.",
	OR,		IOFLAGS,	HEX_TYPE,	NO_TYPE,    "BitOR <0xN> in ioflags.",
	OR,		QFLAGS,		HEX_TYPE,	NO_TYPE,    "BitOR <0xN> in qflags.",
	SET_TIME,	CORRECTION,	INTERVAL_TYPE,	NO_TYPE,    "Set time_correction to <time_interval>.",
	SET_TIME,	HDRTIME,	DATE_TYPE,	NO_TYPE,    "Set hdrtime to <date>.",
	ADD_TIME,	CORRECTION,	INTERVAL_TYPE,	NO_TYPE,    "Add <time_interval> to time_correction.",
	ADD_TIME,	HDRTIME,	INTERVAL_TYPE,	NO_TYPE,    "Add <time_interval> to hdrtime.",
	ADD_TIME,	PCF,		NO_TYPE,	NO_TYPE,    "Add PASSCAL PCF correction to time_correction.",
	GETPCF,		NO_TARGET,	STR_TYPE,	NO_TYPE,    "Read in PASSCAL PCF file.",
	ADD_TREND,	CORRECTION,	INTERVAL_TYPE,	INTERVAL_TYPE, "Add <time_interval>...<time_interval> to time_correction.",
	ADD_TREND,	HDRTIME,	INTERVAL_TYPE,	INTERVAL_TYPE, "Add <time_interval>...<time_interval> to hdrtime.",
	APPLY_CORR,	KEEP_FIELD,	NO_TYPE,	NO_TYPE,    "Apply time_correction to hdr (keep corr and set aflag time bit).",
	APPLY_CORR,	CLEAR_FIELD,	NO_TYPE,	NO_TYPE,    "Apply time_correction to hdr (clear corr and aflag time bit).",
	RESEQUENCE,	NO_TYPE,	NO_TYPE,	NO_TYPE,    "Resequence record, starting with initial sequence #.",
	LIMIT,		CORRECTION,	INT_TYPE,	NO_TYPE,    "Limit time corrections to <n> tick resolution.",
	LIMIT,		MINGAP,		INT_TYPE,	NO_TYPE,    "Limit minimum timegap for tcorr to <N> ticks.",
	LIMIT,		MAXGAP,		INT_TYPE,	NO_TYPE,    "Limit maximum timegap for tcorr to <N> ticks.",
	SMOOTH,		HDRTIME,	NO_TYPE,	NO_TYPE,    "Add time corr to hdrtime to erase clock jitter.",
	SMOOTH,		CORRECTION,	NO_TYPE,	NO_TYPE,    "Add time corr to time_correction to erase clock jitter.",
	TCORR,		HDRTIME,	STR_TYPE,	DATE_TYPE,  "Correct time gaps wrt reference time using hdrtime field.",
	TCORR,		CORRECTION,	STR_TYPE,	DATE_TYPE,  "Correct time gaps wrt reference time using time_correction field.",
	VOLHDR,		ON,		NO_TYPE,	NO_TYPE,    "Create telemetry SEED volhdr if none exists.",
	VOLHDR,		OFF,		NO_TYPE,	NO_TYPE,    "Delete volhdr.",
	NO_OP,		NO_TARGET,	NO_TYPE,	NO_TYPE,
};
#else
extern CMD_TABLE cmd_table[];
#endif

/************************************************************************/

#ifdef	EXTERN
DATATYPE_TABLE datatype_table[] = {
/*----------------------------------------------------------------------*/
/*	Datatype String		Datatype Definition			*/
/*----------------------------------------------------------------------*/
	"<int>",		INT_TYPE,
	"[<int>]",		OPT_INT_TYPE,
	"<hex>",		HEX_TYPE,
	"[<hex>]",		OPT_HEX_TYPE,
	"<string>",		STR_TYPE,
	"[<string>]",		OPT_STR_TYPE,
	"<date/time>",		DATE_TYPE,
	"[<date/time>]",	OPT_DATE_TYPE,
	"<time_interval>",	INTERVAL_TYPE,
	"[<time_interval>]",	OPT_INTERVAL_TYPE,
	NULL,			NO_TYPE,
};
#else
extern DATATYPE_TABLE datatype_table[];
#endif

/************************************************************************/

#ifdef	EXTERN
BOUND_KEYWD_TABLE bound_keywd_table[] = {
/*----------------------------------------------------------------------*/
/*	Bound Keyword String	Bound Keyword Definition		*/
/*----------------------------------------------------------------------*/
	"from",			FROM,
	"to",			TO,
	"thru",			THRU,
	NULL,			NO_OP,
};
#else
extern BOUND_KEYWD_TABLE bound_keywd_table[];
#endif

#ifdef	EXTERN
BOUND_KEYVAL_TABLE bound_keyval_table[] = {
/*----------------------------------------------------------------------*/
/*	Bound Value String	Bound Value Definition		*/
/*----------------------------------------------------------------------*/
	"first",		FIRST_RECORD,
	"prev",			PREVIOUS_RECORD,
	"previous",		PREVIOUS_RECORD,
	"cur",			CURRENT_RECORD,
	"current",		CURRENT_RECORD,
	"last",			LAST_RECORD,
	NULL,			NO_OP,
};
#else
extern BOUND_KEYVAL_TABLE bound_keyval_table[];
#endif

#ifdef	EXTERN
DIRECTION_TABLE direction_table[] = {
/*----------------------------------------------------------------------*/
/*	Direction String	Direction Definition			*/
/*----------------------------------------------------------------------*/
	"forward",		FORWARD_DIR,
	"reverse",		REVERSE_DIR,
	NULL,			NO_OP,
};
#else
extern DIRECTION_TABLE direction_table[];
#endif

/************************************************************************/
#ifndef	EXTERN
#define	EXTERN
#endif

/************************************************************************/
/*  Command line information.						*/
/*----------------------------------------------------------------------*/
/*  Command line options.						*/
EXTERN int	start_flag;		/* starting time specified.	*/
EXTERN int	end_flag;		/* ending time specified.	*/

/*  Command line arguments.						*/
EXTERN char	*cmdname;		/* command name.		*/
EXTERN char	*infile;		/* input file name.		*/
EXTERN int	debug_option;		/* debug options.		*/
EXTERN int	date_fmt;		/* format for printing date.	*/
EXTERN INT_TIME	start_time;		/* starting time for selection.	*/
EXTERN INT_TIME	end_time;		/* ending time for selection.	*/

/*  Misc external info.							*/
EXTERN FILE	*fpin;			/* input FILE pointer.		*/
EXTERN FILE	*fpout;			/* output FILE pointer.		*/
EXTERN FILE	*info;			/* information FILE pointer.	*/

EXTERN int	herrno;			/* errno from header routines.	*/
EXTERN char	*outfile;		/* output file name.		*/
EXTERN ST_INFO	st_head_in;		/* hdr struture to linked list	*/
					/* of input stream structures.	*/
EXTERN ST_INFO	st_head_out;		/* hdr struture to linked list	*/
					/* of output stream structures.	*/

EXTERN int	auto_display;		/* display hdr automatically	*/
					/* after every command that	*/
					/* changes hdr or definition	*/
					/* of current hdr.		*/
EXTERN int	no_auto_display;	/* do not autodisplay this time.*/
EXTERN int	extended_bounds;	/* flag that determines whether	*/
					/* boundaries of "from" date	*/
					/* can exceed file boundaries.	*/
EXTERN int	interactive;		/* is input from a tty?		*/
extern char	*syntax[];		/* command line syntax info.	*/
/************************************************************************/
