/**
 * HitMap library main include file.
 *
 * This header file includes the header files needed to use all the functionalities of HitMap.
 * It is the only header file to be used in all Hitmap applications.
 *
 * @file hitmap2.h
 * @version 2.0
 * @author Arturo Gonzalez-Escribano
 * @date Oct 2017
 *
 */

/*
 *
 * @section License Hitmap license
 * <license>
 * 
 * Hitmap v2.0
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
 * Copyright (c) 2007-2017, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * More information on http://trasgo.infor.uva.es/
 * 
 * </license>
 *
 */

#ifndef _HITMAP2_
#define _HITMAP2_

#include "hitmap.h"

/**
 * Version of this Hitmap library release
 */
#undef	HITMAP_VERSION
#define HITMAP_VERSION	2.0

/**
 * Macros for the new v2.0 interface
 */


#define HIT2_COUNTPARAM( ... )	HIT2_COUNTPARAM_N( __VA_ARGS__, 20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1 )
#define HIT2_COUNTPARAM_N( n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16, n17, n18, n19, n20, num, ... )	num

/**
 * New Tile access macros
 */
#define hit(var, ...)	HIT2_HIT_VARIADIC( var, HIT2_COUNTPARAM( __VA_ARGS__ ), __VA_ARGS__ )
#define HIT2_HIT_VARIADIC(var, num, ...)	hit_tileElemAtNoStride( var, num, __VA_ARGS__ )

#define hitStrided(var, ...)	HIT2_HITSTRIDED_VARIADIC( var, HIT2_COUNTPARAM( __VA_ARGS__ ), __VA_ARGS__ )
#define HIT2_HITSTRIDED_VARIADIC(var, num, ...)	hit_tileElemAt( var, num, __VA_ARGS__ )

/**
 * New Shape constructor macro
 */
#define hitNewShape( ... )	HIT2_SHAPE_VARIADIC( HIT2_COUNTPARAM( __VA_ARGS__ ), __VA_ARGS__ )
#define HIT2_SHAPE_VARIADIC( num, ... )	HIT2_SHAPE_VARIADIC_2( num, __VA_ARGS__ )
#define HIT2_SHAPE_VARIADIC_2( num, ... )	hit_shape( num, HIT2_HITSIGS_##num( __VA_ARGS__ ) )
#define HIT2_HITSIGS_1( tuple )	hitNewSig tuple
#define HIT2_HITSIGS_2( tuple, ... )	hitNewSig tuple, HIT2_HITSIGS_1( __VA_ARGS__ )
#define HIT2_HITSIGS_3( tuple, ... )	hitNewSig tuple, HIT2_HITSIGS_2( __VA_ARGS__ )
#define HIT2_HITSIGS_4( tuple, ... )	hitNewSig tuple, HIT2_HITSIGS_3( __VA_ARGS__ )

/**
 * New Signature constructor macro
 */
#define hitNewSig( ... )	HIT2_HITSIG_VARIADIC( HIT2_COUNTPARAM(__VA_ARGS__), __VA_ARGS__ )
#define HIT2_HITSIG_VARIADIC( num, ... )	HIT2_HITSIG_VARIADIC_2( num, __VA_ARGS__ )
#define HIT2_HITSIG_VARIADIC_2( num, ... ) 	hitSig_##num( __VA_ARGS__ )
#define hitSig_1( a )		hit_sig( a, a, 1 )
#define hitSig_2( a, b )	hit_sig( a, b, 1 )
#define hitSig_3( a, b, c )	hit_sig( a, b, c )

/**
 * New Tile Type definition including a specialized static inline constructor
 */
#define hitNewType( type ) 	hit_tileNewType( type );	\
	static inline HitTile_##type hitNewTile_##type( HitShape shp ) {				\
		HitTile_##type new = HIT_TILE_NULL_STATIC;									\
		hit_tileDomainShapeAlloc( &new, type, shp );								\
		return new;																	\
	}																			

/**
 * Generic local tile constructor
 */
#define hitNewTile( type, shape )	hitNewTile_##type( shape )

/**
 * Distributed tiles
 * ...
 */

#endif // HITMAP v2.0

