/**
 * Types and functions to manipulate bitmap sparse domains.
 *
 * @file hit_bshape.c
 * @version 1.0
 * @author Javier Fresno Bausela
 * @date Nov 2011
 */

/*
 * <license>
 * 
 * Hitmap v1.4
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
 * Copyright (c) 2007-2024, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * More information on http://trasgo.infor.uva.es/
 * 
 * </license>
*/

#include <string.h>
#include <hit_bshape.h>
#include <hit_allocP.h>
#include <hit_funcop.h>


HitShape hit_bitmapShape(HitInd nvertices){

	// 1. Init the Bitmap struct
	HitShape s = HIT_BITMAP_SHAPE_NULL;
	hit_bShapeCard(s,0) = nvertices;
	hit_bShapeCard(s,1) = nvertices;
	
	// 2. Allocate the square adyacency matrix
	HitInd nbits = nvertices*nvertices;
	size_t ndata = (size_t) (hit_bitmapShapeIndex(nbits) + (hit_bitmapShapeOffset(nbits)==0 ? 0 : 1));
	// @arturo Ago 2015: New allocP interface
	// hit_calloc(hit_bShapeData(s),sizeof(HIT_BITMAP_TYPE),HIT_BITMAP_TYPE*);
	hit_calloc(hit_bShapeData(s),HIT_BITMAP_TYPE,ndata);
	
	// 3. Allocate and fill the global names array
	hit_nameListCreate(&hit_bShapeNameList(s,0),nvertices);
	hit_bShapeNameList(s,1) = hit_bShapeNameList(s,0);
	
	// 4. Return the struct
	return s;
}


HitShape hit_bitmapShapeMatrix(HitInd n, HitInd m){

	HitShape s = HIT_BITMAP_SHAPE_NULL;

	// Cardinalities.
	hit_bShapeCard(s,0) = n;
	hit_bShapeCard(s,1) = m;

	// Allocate the arrays.
	HitInd nbits = n * m;
	size_t ndata = (size_t) (hit_bitmapShapeIndex(nbits) + (hit_bitmapShapeOffset(nbits)==0 ? 0 : 1));
	// @arturo Ago 2015: New allocP interface
	// hit_calloc(hit_bShapeData(s),ndata+1,sizeof(HIT_BITMAP_TYPE),HIT_BITMAP_TYPE*);
	hit_calloc(hit_bShapeData(s),HIT_BITMAP_TYPE,ndata+1);

	// This allows the bitmap iterator to stop at the end if the last bits are 0.
	hit_bShapeData(s)[ndata] = 1;

	hit_nameListCreate(&hit_bShapeNameList(s,0),n);
	hit_nameListCreate(&hit_bShapeNameList(s,1),m);

	// Return the shape
	return s;
}


void hit_bShapeFree(HitShape shape){

	hit_free(hit_bShapeData(shape));
	hit_nameListFree(hit_bShapeNameList(shape,0));
	
	if(hit_bShapeNameList(shape,0).names != hit_bShapeNameList(shape,1).names)
		hit_nameListFree(hit_bShapeNameList(shape,1));
}


/**
 * Copy the elements from a bitmap shape to another.
 * @param dst Destination shape.
 * @param src Source shape.
 * @note src should contain dst.
 */
void hit_bShapeCopyElementsInternal(HitShape dst, HitShape src){

	for(HitInd i=0; i < hit_bShapeCard(dst,0); i++){
		for(HitInd j=0; j < hit_bShapeCard(dst,1); j++){

			// For each element of the bitmap we get the global names
			HitInd g_i = hit_nameListIndex2Name( hit_bShapeNameList(dst,0), i);
			HitInd g_j = hit_nameListIndex2Name( hit_bShapeNameList(dst,1), j);

			// Check if the source shape has those elements.
			if(hit_bShapeGetGlobal(src,g_i,g_j)){

				hit_bShapeSet(dst,i,j);
				hit_bShapeNedges(dst)++;
			}
		}
	}
}



HitShape hit_bShapeSelect(HitShape shape, HitInd nvertices, HitInd * vertices){

	if(nvertices < 1) return HIT_BITMAP_SHAPE_NULL;
	
	// 1. If vertices is NULL we use the name vector (a.k.a. n first vertices)
	if(vertices == NULL){
		vertices = hit_bShapeNameList(shape,0).names;
	}
	
	// 2. Create a new graph with edge array as big as the original.
	HitShape res = hit_bitmapShape(nvertices);
	
	// 3. Duplicate vertices array as globalNames.
	memcpy(hit_bShapeNameList(res,0).names, vertices, sizeof(HitInd) * (size_t) nvertices );
	hit_bShapeNameList(res,0).nNames = nvertices;
	hit_bShapeNameList(res,0).flagNames = HIT_SHAPE_NAMES_NOARRAY;
	hit_bShapeNameList(res,1) = hit_bShapeNameList(res,0);
	
	// Copy the Elements from the original shape to the result shape.
	hit_bShapeCopyElementsInternal(res,shape);
	
	// 8. Return
	return res;
}




HitShape hit_bShapeSelectRows(HitShape shape, HitInd n, HitInd * names){

	// Select cardinalities: matrix of n x m
	HitInd m = hit_bShapeCard(shape,1);

	// Create the matrix
	HitShape res = hit_bitmapShapeMatrix(n,m);

	// Create the name lists
	memcpy(hit_bShapeNameList(res,0).names, names, (size_t) n *sizeof(idxtype) );
	hit_bShapeNameList(res,0).flagNames = HIT_SHAPE_NAMES_NOARRAY;
	hit_nameListFree(hit_bShapeNameList(res,1));
	hit_nameListClone( &hit_bShapeNameList(res,1),  &hit_bShapeNameList(shape,1) );

	// Copy the values to the new shape
	hit_bShapeCopyElementsInternal(res,shape);

	return res;
}











HitShape hit_bShapeExpand(HitShape shape, HitShape original, int amount){

	if( hit_bShapeNvertices(shape) < 1) return HIT_BITMAP_SHAPE_NULL;

	// 1. Create a vertex list
	HitInd orig_nvertices = hit_bShapeNvertices(original);
	HitInd * vertices;
	// @arturo Ago 2015: New allocP interface
	// hit_calloc(vertices, (size_t)orig_nvertices, sizeof(int),int*);
	hit_calloc(vertices, HitInd, orig_nvertices);
	
#define VLOCAL	1 // (....0001)
#define VEXPAND	2 // (....0010)

	// 2. Set the local vertices in the vector.
	{
		HitInd vertex;
		hit_bShapeVertexIterator(vertex,shape){
		
			HitInd global_vertex = hit_bShapeVertexToGlobal(shape,vertex);
			HitInd local_orig_vertex = hit_bShapeVertexToLocal(original,global_vertex);
			
			vertices[local_orig_vertex] = VLOCAL;
		}
	}
	
	// 3. Expand the list.
	HitInd new_vertices = 0;
	for(int a=0; a<amount; a++){
		
		// Complete the local vertices and the number of vertices.
		for(HitInd vertex_i=0; vertex_i<orig_nvertices; vertex_i++){
			
			for(HitInd vertex_j=0; vertex_j<orig_nvertices; vertex_j++){
				
				if(hit_bShapeGet(original,vertex_i,vertex_j) == 0) continue;
				
				// TO
				if((vertices[vertex_i] & VLOCAL) && (! vertices[vertex_j] & VLOCAL)){
					vertices[vertex_j] = VEXPAND;
					new_vertices++;
				}
				
				// @javfres @todo The hit_bShapeExpand function expand the shape
				// following the edges from source to target. I call this TO direction.
				// It can be interesting to add a FROM option, or maybe several functions
				// with TO/FROM/TOFROM.
			}

		}
		
		// Set the expanded vertices as local for the next loop.
		if(a<amount-1){
			for(HitInd vertex=0; vertex<orig_nvertices; vertex++){
				if(vertices[vertex] == VEXPAND) vertices[vertex] |= VLOCAL;
			}
		}
		
	}
	
	// 3. Create selection list
	HitInd * vlist;
	HitInd nvlist = new_vertices + hit_bShapeNvertices(shape);
	// @arturo Ago 2015: New allocP interface
	// hit_malloc(vlist,sizeof(int) * (size_t) nvlist,int*);
	hit_malloc(vlist, HitInd, nvlist);
	
	// 4. The first vertices of the old and new shape are the same.
	memcpy(vlist,hit_bShapeNameList(shape,0).names, sizeof(HitInd) * (size_t) hit_bShapeNvertices(shape));
	
	// 5. Complete the selection list.
	HitInd vertex_index = hit_bShapeNvertices(shape);
	for(HitInd vertex=0; vertex<orig_nvertices; vertex++){
	
		if(vertices[vertex] > VLOCAL){
			vlist[vertex_index] = hit_bShapeVertexToGlobal(original,vertex);
			vertex_index++;
		}
	}
	
	// 6. Select
	HitShape res = hit_bShapeSelect(original,nvlist,vlist);
	
	
	// 7. Free resources.
	hit_free(vertices);
	hit_free(vlist);
	
	
#undef VLOCAL
#undef VEXPAND
	
	return res;

}



void hit_bShapeAddEmptyRow_or_Vertex(HitShape * shape, HitInd x, int mode){

#define s (*shape)

	HitInd local_x;

	local_x = hit_bShapeCoordToLocal(s,0,x);

	if( local_x  != -1 ) return;
	local_x = hit_bShapeCard(s, 0);

	// A. Create the sparse shape.
	if(local_x == 0){

		// A.1 Create new bitmap shape.
		if(mode == HIT_BSHAPE_GRAPH){
			s = hit_bitmapShape(1);
		} else {
			s = hit_bitmapShapeMatrix(1,0);
		}
		hit_bShapeNameList(s,0).names[0] = x;


	// B. The given sparse shape is not empty.
	} else {

		// B.1 Create new bitmap shape.
		HitInd nelems_x = hit_bShapeCard(s,0) + 1;
		HitInd nelems_y = hit_bShapeCard(s,1);
		HitShape newShp;

		if (mode == HIT_BSHAPE_GRAPH){
			newShp = hit_bitmapShape(nelems_x);
		} else {
			newShp = hit_bitmapShapeMatrix(nelems_x,nelems_y);
		}

		// B.2 Copy the data
		for(HitInd i=0; i<(nelems_x-1); i++){
			for(HitInd j=0; j<(nelems_y); j++){
				if(hit_bShapeGet(s,i,j)){
					hit_bShapeSet(newShp,i,j);
				}
			}
		}

		// B.3 Free the previous data and copy the new to the previous shape.
		hit_free(hit_bShapeData(s));
		hit_bShapeData(s) = hit_bShapeData(newShp);

		// B.4 Update the cardinalities.
		hit_bShapeCard(s,0) = hit_bShapeCard(newShp,0);
		hit_bShapeCard(s,1) = hit_bShapeCard(newShp,1);

		// B.5 Free the new name vectors
		hit_nameListFree(hit_bShapeNameList(newShp,0));
		if (mode != HIT_BSHAPE_GRAPH){
			hit_nameListFree(hit_bShapeNameList(newShp,1));
		}

		// B.6 Update the name vectors
		hit_nameListAdd(&hit_bShapeNameList(s,0),x);
		if (mode == HIT_BSHAPE_GRAPH){
			hit_bShapeNameList(s,1) = hit_bShapeNameList(s,0);
		}
	}


#undef s

}




void hit_bShapeAddColumn(HitShape * shape, HitInd y){

#define s (*shape)


	if(hit_nameListName2Index(hit_bShapeNameList(s,1),y) == -1){

		// 1 Create new bitmap shape.
		HitInd nelems_x = hit_bShapeCard(s,0);
		HitInd nelems_y = hit_bShapeCard(s,1) + 1;
		HitShape newShp = hit_bitmapShapeMatrix(nelems_x,nelems_y);

		// 2 Copy the data
		for(HitInd i=0; i<(nelems_x); i++){
			for(HitInd j=0; j<(nelems_y-1); j++){
				if(hit_bShapeGet(s,i,j)){
					hit_bShapeSet(newShp,i,j);
				}
			}
		}

		// 3 Free the previous data and copy the new to the previous shape.
		hit_free(hit_bShapeData(s));
		hit_bShapeData(s) = hit_bShapeData(newShp);

		// 4 Update the cardinalities.
		hit_bShapeCard(s,1)++;

		// 5 Free the new name vectors
		hit_nameListFree(hit_bShapeNameList(newShp,0));
		hit_nameListFree(hit_bShapeNameList(newShp,1));

		// 6 Update the name vectors
		hit_nameListAdd(&hit_bShapeNameList(s,1),y);
	}

#undef s

}




void hit_bShapeAddElem_or_Edge(HitShape * shape, int x, int y, int mode){

	// 1. Add the vertices
	hit_bShapeAddEmptyRow_or_Vertex(shape, x, mode);
	if(mode == HIT_BSHAPE_GRAPH){
		hit_bShapeAddEmptyRow_or_Vertex(shape, y, mode);
	} else {
		hit_bShapeAddColumn(shape, y);
	}
	
	// 2. Add the edge
	if(hit_bShapeGetGlobal(*shape,x,y) == 0){
		hit_bShapeSetGlobal(*shape,x,y);
		hit_bShapeNedges(*shape)++;
	}

}




HitInd hit_bShapeNColsRow(HitShape shape, HitInd row){

	HitInd ncols = 0;
	HitInd j;

	hit_bShapeColumnIterator(j,shape,row){
		ncols++;
	}

	return ncols;
}



