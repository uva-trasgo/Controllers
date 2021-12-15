#include "utilities.h"

int hit_tileNumElem(HitTile *m){
  int numElems;
  HitShape tile_m = hit_tileShape(*m);
  if(hit_shapeDims(tile_m)==1){
    numElems=hit_shapeSigCard(tile_m,0) ;
    }
  if(hit_shapeDims(tile_m)==2){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1);
    }
  if(hit_shapeDims(tile_m)==3){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1)*hit_shapeSigCard(tile_m,2);
    }
  if(hit_shapeDims(tile_m)==4){
    numElems=hit_shapeSigCard(tile_m,0)*hit_shapeSigCard(tile_m,1)*hit_shapeSigCard(tile_m,2)*hit_shapeSigCard(tile_m,3);
    }

  return numElems;
}

