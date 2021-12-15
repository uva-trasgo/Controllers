/**
 * Hitmap pattern definition.
 * A pattern structure includes one or more communications.
 * These communications can be performed ordered or unordered.
 * 
 * @file hit_pattern.h
 * @version 1.3
 * @author Arturo Gonzalez-Escribano
 * @author Javier Fresno Bausela
 * @author Carlos de Blas Carton
 * @date Mar 2013
 *
 */

/*
 * <license>
 * 
 * Hitmap v1.2
 * 
 * This software is provided to enhance knowledge and encourage progress in the scientific
 * community. It should be used only for research and educational purposes. Any reproduction
 * or use for commercial purpose, public redistribution, in source or binary forms, with or 
 * without modifications, is NOT ALLOWED without the previous authorization of the copyright 
 * holder. The origin of this software must not be misrepresented; you must not claim that you
 * wrote the original software. If you use this software for any purpose (e.g. publication),
 * a reference to the software package and the authors must be included.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Copyright (c) 2007-2015, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * More information on http://trasgo.infor.uva.es/
 * 
 * </license>
*/

#ifndef _HitPattern_
#define _HitPattern_

#include <stdio.h>
#include "hit_com.h"

#ifdef __cplusplus
	extern "C" {
#endif

/* Hit COMMUNICATION PATTERN STRUCTURE */
/**
 * Hit communication pattern structure.
 * This structure is a linked list.
 */
typedef struct HitPatS {
	HitCom			commSpec;	/**< HitCom communicator. */
	struct HitPatS	*next;		/**< Pointer to the next element. */
} HitPatS;


/**
 * Structure for patterns.
 * This structure is the head of the linked list.
 */
typedef struct {
	int numComm;		/**< Number of HitCom objects in the list. */
	int defaultMode;	/**< Ordered or Unordered. */
	HitPatS	*first;		/**< Fist element of the list. */
	HitPatS	*last;		/**< Last element of the list. */
} HitPattern;

/** ordered pattern */
#define	HIT_PAT_ORDERED		0
/** unordered pattern */
#define	HIT_PAT_UNORDERED	1

/* Hit PATTERN: NULL VALUE */
#define HIT_PATTERN_NULL_STATIC	{ 0, HIT_PAT_ORDERED, NULL, NULL }
extern HitPattern HIT_PATTERN_NULL;

/* Hit PATTERN: INITIALIZE */
/**
* creation of an empty pattern
* @param defaultMode constant to indicate if the pattern is going to be executed orderly or not
* @result new empty pattern formed
*/
static inline HitPattern hit_pattern( int defaultMode ) {
	HitPattern newPat = { 0, defaultMode, NULL, NULL };
	return newPat;
}

/* Hit PATTERN: ADD COMM */
/**
* Add a communication to a pattern
* @param pattern pointer to the old pattern
* @param comm the communication
* @result the new pattern formed by the communications in pattern and the new communication
*/
void hit_patternAdd( HitPattern *pattern, HitCom comm );

/* Hit PATTERN: COMPOSE PATTERN */
/**
* Add a whole pattern to another pattern
* @param pattern pointer to the pattern to compose that will also be the result
* @param pattern2 pointer to the pattern to compose that will not be the result
* @result the new pattern formed by the communications in pattern and the new communication
*/
void hit_patternCompose( HitPattern *pattern, HitPattern *pattern2 );


/* Hit PATTERN: FREE */
/**
* Free resources in a pattern. It cannot be used again until a new creation
* @param pattern pointer to the pattern
*/
void hit_patternFree( HitPattern *pattern );

/* Hit PATTERN: EXECUTE ORDERED */
/**
* Do all the communications in a pattern orderly according to the order of their adding to the pattern
* @param pattern pointer to the pattern
*/
void hit_patternDoOrdered( HitPattern pattern );

/* Hit PATTERN: EXECUTE UNORDERED */
/**
* Do all the communications of a pattern in a unordered way.
* @param pattern pointer to the pattern
*/
void hit_patternDoUnordered( HitPattern pattern );

/* Hit PATTERN: START ASYNC SEND/RECV */
/**
* Start all asynchronous communications in a pattern
* @param pattern pointer to the pattern
*/
void hit_patternStartAsync( HitPattern pattern );

/* Hit PATTERN: END ASYNC SEND/RECV */
/**
* End all asynchronous communications in a pattern
* @param pattern pointer to the pattern
*/
void hit_patternEndAsync( HitPattern pattern );

/* Hit PATTERN: EXECUTE IN DEFAULT MODE */
/**
* End (wait) all asynchronous communications in a pattern
* @param pattern pointer to the pattern
*/
#define hit_patternDo( pattern )		{		\
	if ( pattern.defaultMode == HIT_PAT_ORDERED ) 	\
		hit_patternDoOrdered( pattern );		\
	else											\
		hit_patternDoUnordered( pattern );	\
	}


/* Hit PATTERN: EXECUTE AND FREE A PATTERN */
/**
 * hit_patternDoOnce: execute a comm. pattern and free the pattern object.
 * @param com A pattern object (or a pattern constructor)
 */
#define	hit_patternDoOnce( pat )	{ HitPattern __HIT_PAT__ = pat; hit_patternDo( __HIT_PAT__ ); hit_patternFree( &__HIT_PAT__ ); }



/* Hit PATTERN: SPECIFIC PATTERNS FOR SPECIAL COMPLEX COMMUNICATIONS */
/**
 * @arturo Feb 2013
 * hit_pattternLayRedistribute: Redistribute tile parts using two layouts
 * @param lay1 Original layout
 * @param lay2 New layout
 * @param tileP1 Pointer to HitTile allocated on each processor according to the original layout
 * @param tileP2 Pointer to HitTile allocated on each processor according to the second layout
 * @param baseType HitType of the base elements of the tiles
 * @return HitPattern communication structure.
 */
HitPattern hit_patternLayRedistribute( HitLayout lay1, HitLayout lay2, void * tileP1, void *tileP2, HitType baseType );

/**
 * @arturo Feb 2013
 * hit_pattternRedistribute: Redistribute tile parts using their shapes
 * @param lay1 Layout object for the communication
 * @param tileP1 Pointer to HitTile from which to get data
 * @param tileP2 Pointer to HitTile in which to receive data
 * @param baseType HitType of the base elements of the tiles
 * @return HitPattern communication structure.
 */
HitPattern hit_patternRedistribute( HitLayout lay, void * tileP1, void *tileP2, HitType baseType, int flagCompact );


/**
 * Creates a pattern to communicate data for the matrix-vector benchmark.
 * CSR Version.
 * @param pattern Pointer to the pattern.
 * @param lay The layout.
 * @param origin_matrix The shape of the original matrix.
 * @param matrix The shape of the local matrix.
 * @param tilePSend The local vector to send.
 * @param tilePRecv The local vector to recv.
 * @param baseType The basetype.
 */
#define hit_patMatMult(pattern, lay, origin_matrix, matrix, tilePSend, tilePRecv, baseType) hit_patMatMultInternal(pattern, lay, origin_matrix, matrix, tilePSend, tilePRecv, baseType,__FILE__, __LINE__);
void hit_patMatMultInternal(HitPattern *pattern, HitLayout lay, HitShape origin_matrix, HitShape matrix, const void * tilePSend, const void * tilePRecv, HitType baseType, const char * file, int line);


/**
 * Creates a pattern to communicate data for the matrix-vector benchmark.
 * Bitmap version.
 * @param pattern Pointer to the pattern.
 * @param lay The layout.
 * @param origin_matrix The shape of the original matrix.
 * @param matrix The shape of the local matrix.
 * @param tilePSend The local vector to send.
 * @param tilePRecv The local vector to recv.
 * @param baseType The basetype.
 */
#define hit_patMatMultBitmap(pattern, lay, origin_matrix, matrix, tilePSend, tilePRecv, baseType) hit_patMatMultBitmapInternal(pattern, lay, origin_matrix, matrix, tilePSend, tilePRecv, baseType,__FILE__, __LINE__);
void hit_patMatMultBitmapInternal(HitPattern *pattern, HitLayout lay, HitShape origin_matrix, HitShape matrix, const void * tilePSend, const void * tilePRecv, HitType baseType, const char * file, int line);









#ifdef __cplusplus
}
#endif

/* END OF HEADER FILE _HitPattern_ */
#endif
