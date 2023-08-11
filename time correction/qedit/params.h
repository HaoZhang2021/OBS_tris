/************************************************************************/
/*  Definitions for qedit.						*/
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

/*	$Id: params.h,v 1.9 2009/07/08 20:41:24 doug Exp $ 	*/

#define	    DEBUG
#define	    DEBUG_CMD		1
#define	    DEBUG_EXECUTE	2	
#define	    DEBUG_DO		4
#define	    DEBUG_DETAIL	8
#define	    DEBUG_ANY	\
    (DEBUG_CMD | DEBUG_EXECUTE | DEBUG_DO | DEBUG_DETAIL )
#define	    debug(val)  (debug_option & (val))

#define	    FRAMESIZE		64
#define	    MAX_BLKSIZE		8192
#define	    BUFSIZE		(2*MAX_BLKSIZE)

/*  TOL(rate) = max size of gap in ticks that can be tolerated between	*/
#define	TOL(rate,mult)		((.4*TICKS_PER_SEC)/(sps_rate(rate,mult)))
