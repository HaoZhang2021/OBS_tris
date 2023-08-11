/************************************************************************/
/*  Structure definitions for qedit.					*/
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

/*	$Id: defs.h,v 1.10 2004/01/21 03:50:23 doug Exp $ 	*/

#define	FALSE		0
#define	TRUE		1

#define	MATCH_STR(s1,s2)	(s1!=NULL && s2!=NULL && strcasecmp(s1,s2)==0)
#define	MATCH_VAL(v1,v2)	(v1==v2)
#define	NULL_STR		"(null)"

/************************************************************************/

typedef enum {
	NO_OP, FIND, FIRST, LAST, FORWARD, BACK, DISPLAY, ADD, SET, CLEAR, 
	AND, OR, DELETE, SET_TIME, ADD_TIME, ADD_TREND, AUTODISPLAY, 
	QUIT, FILEQUIT, HELP, VERIFY, WRITE, BOUNDS, APPLY_CORR, RESEQUENCE,
	LIMIT, SMOOTH, TCORR, GETPCF, VOLHDR,
} OP;

typedef enum {
	NO_TYPE, INT_TYPE, HEX_TYPE, STR_TYPE, INTERVAL_TYPE, DATE_TYPE,
	OPT_INT_TYPE, OPT_HEX_TYPE, OPT_STR_TYPE, OPT_INTERVAL_TYPE,
	OPT_DATE_TYPE,
} DATATYPE;

typedef enum {	
	NO_TARGET, STATION, LOCATION, CHANNEL, NETWORK, SEQNO, TEAR, NSAMPLES,
	RATE_FACTOR, RATE_MULT,	RATE, AFLAGS, IOFLAGS, QFLAGS, X0, XN,
	CORRECTION, HDRTIME, BEGTIME, TIME, RECORD, RECNO, BLOCK, RECORDTYPE,
	HEADER, STREAM, ON, OFF, ALL, KEEP_FIELD, CLEAR_FIELD, REFTIME, MINGAP,
	MAXGAP, PCF, 
} TARGET;
    
typedef enum {
	NO_KEYWD, FROM, TO, THRU,
} BOUND_KEYWD;

typedef enum {
	NO_KEYVAL, FIRST_RECORD, PREVIOUS_RECORD, CURRENT_RECORD, LAST_RECORD,
} BOUND_KEYVAL;

typedef enum {
	NO_DIR, REVERSE_DIR, FORWARD_DIR, 
} DIRECTION;

/************************************************************************/

typedef struct _op_table {
	char		*string;
	OP		op;
} OP_TABLE;

/************************************************************************/

typedef struct _target_table {
	char		*string;
	TARGET		target;
} TARGET_TABLE;

/************************************************************************/

typedef struct _direction_table {
	char		*string;
	DIRECTION	direction;
} DIRECTION_TABLE;

/************************************************************************/

typedef struct _datatype_table {
	char		*string;
	DATATYPE	datatype;
} DATATYPE_TABLE;

/************************************************************************/

typedef struct _cmd_table {
	OP		op;
	TARGET		target;
	DATATYPE	arg_0;
	DATATYPE	arg_1;
	char		*description;
} CMD_TABLE;
	
/************************************************************************/

typedef struct _arg {
	DATATYPE	argt;
	union {
		char		*str;
		int		ival;
		int		hval;
		INT_TIME	*time;
		EXT_TIME	*interval;
	} argv;
} ARG;

typedef struct _cmd {
	OP		op;
	TARGET		target;
	ARG		arg[2];
	ARG		from;
	ARG		to;
	BOUND_KEYWD	ub;
} CMD;

/************************************************************************/

typedef struct _bound_keywd_table {
	char		*string;
	BOUND_KEYWD	key;
} BOUND_KEYWD_TABLE;

typedef struct _bound_keyval_table {
	char		*string;
	BOUND_KEYVAL	key;
} BOUND_KEYVAL_TABLE;

/************************************************************************/

typedef struct _blkinfo {
	struct _blkinfo	*next;
	struct _blkinfo	*prev;
	int		blk_no;
	DATA_HDR	*hdr;
	char		*data;
	int		datalen;
} BLKINFO;

