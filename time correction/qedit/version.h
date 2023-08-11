/************************************************************************/
/*  Version number.							*/
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

/*	$Id: version.h,v 1.23 2009/07/08 20:41:24 doug Exp $ 	*/

#define	VERSION	"1.4.21 (2009.189)"

/*
Modification History
------------------------------------------------------------------------
Date	    Ver			Who What
------------------------------------------------------------------------
2009/07/08	    1.4.21 (2009.189)	Doug Nehauser
	Changed MAX_BLKSIZE from 4096 to 8192.
2005/12/13  1.4.20 (2005.347)	Doug Neuhauser
	Fix "find tear" to not wrap from bottom to top of file.
2005/10/16  1.4.19 (2005.289)	Doug Neuhauser
	Fix limit corr command.
2005/09/04  1.4.18 (2005.247)	Doug Neuhauser
	Fix limit command (was processing all limits commands as limit corr).
2005/06/29  1.4.17 (2005.180)	Doug Neuhauser
	Removed text after "#endif" in get_rate.h
2005/03/21  1.4.16 (2005.080)	Doug Neuhauser
	execute.c
	Fix iteration over "del" command.
2005/01/08  1.4.15 (2005.008)	Doug Neuhauser
	parse_cmd.c
	Explicit null pointer for ARG structure to null for empty optional arg.
2004/11/07  1.4.14 (2004.312)	Doug Neuhauser
	do.c
	Allow value of "-" or "--" for blank string in verify command.
2004/03/06  1.4.13 (2004.066)	Doug Neuhauser (per ISTI)
	get_rate.c
	Change cfree() to free() for MACOSX support.
2004/02/24  1.4.12 (2004.055)	Doug Neuhauser
	do.c
	Fix: Allow value of  "-" or "--" to convert to blank string for
	network or location code.
2004/02/06  1.4.11 (2004.037)	Doug Neuhauser
	do.c
	Allow value of  "-" or "--" to convert to blank string for
	network or location code.
2004/01/20  1.4.10 (2004.020)	Doug Neuhauser
	do.c, execute.c, externals.h, defs.h
	Added volhdr command to create and delete volhdr.
2003/07/11  1.4.9 (2003.192)	Doug Neuhauser
	qedit.c, externals.h, do.c, bounds.c, st_info.h, defs.h, write_file.c
	Added previous as previous position pointer in stream.
	Added cur and prev as synonyms for current an previous in interation.
	Fixed missing argument to strncpy call in write_file_drm().
2002/11/22  1.4.8 (2002.326)	Doug Neuhauser
	bounds.c, defs.h, version.h, externals.h, do.c
	Added commands set rectype, verify rectype.
	Added rectype to display.
	Added commands find rec, find recno as aliases for find blk.
	Changed wording from block to record.
2002/04/09  1.4.7 (2002.099)	Doug Neuhauser
	write_file.c, version.h
	Restore current block position after write.
2000/20/19  1.4.6 (2001.264)	Doug Neuhauser
	do.c, version.h, 
	Added details of blockette 100.
2000/20/19  1.4.5 (2000.293)	Doug Neuhauser
	do.c, execute.c, qedit.c, parse_cmd.c,
	defs.h, externals.h, params.h, procs.h, version.h, 
	Added commands: limit mingap, limit maxgap, and tcorr.
2000/01/04  1.4.4 (2000.004)	Doug Neuhauser
	do.c, version.h
	Added display of fields in blockette 2000.
1999/08/02  1.4.3 (1999.214)	Doug Neuhauser
	do.c, version.h
	Added display of mshear bit in b1001 flags.
1999/02/02  1.4.2 (1999.061)	Doug Neuhauser
	Fix parse_cmdline.c to allow output file to be specified on cmdline.
1997/11/08  1.4.1 (1997.312)	Doug Neuhauser
	Added option of building qedit with GNU readline.
*/
