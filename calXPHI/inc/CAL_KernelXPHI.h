//#ifndef CAL_KERNELXPHI_H
#define CAL_KERNELXPHI_H

#include <omp.h>
#include "CAL_KernelCommon.h"


#define ALLOC   alloc_if(1)
#define FREE    free_if(1)
#define RETAIN  free_if(0)
#define REUSE   alloc_if(0)


#if __ICC

static void  __attribute__((target(mic))) hit_xtile(HitXTile *result, HitTile *ref, float *data){
//        HitTile         *ref = (HitTile *)refP;
        (*result).data= (float*) data;
        (*result).origAcumCard[0] = (*ref).origAcumCard[0];
        (*result).origAcumCard[1] = (*ref).origAcumCard[1];
        (*result).origAcumCard[2] = (*ref).origAcumCard[2];
        (*result).origAcumCard[3] = (*ref).origAcumCard[3];
        (*result).card[0]= (*ref).card[0];
        (*result).card[1]= (*ref).card[1];
        (*result).card[2]= (*ref).card[2];
        (*result).qstride[0]=1;// (*ref).qstride[0];
        (*result).qstride[1]=1; // (*ref).qstride[1];
        (*result).qstride[2]=1;// (*ref).qstride[2];

}
static void  hit_copyTile(HitTile *result, HitTile *ref){
//        HitTile         *ref = (HitTile *)refP;
        (*result).data= (*ref).data;
        (*result).origAcumCard[0] = (*ref).origAcumCard[0];
        (*result).origAcumCard[1] = (*ref).origAcumCard[1];
        (*result).origAcumCard[2] = (*ref).origAcumCard[2];
        (*result).origAcumCard[3] = (*ref).origAcumCard[3];
        (*result).card[0]= (*ref).card[0];
        (*result).card[1]= (*ref).card[1];
        (*result).card[2]= (*ref).card[2];
        (*result).qstride[0]=1;// (*ref).qstride[0];
        (*result).qstride[1]=1; // (*ref).qstride[1];
        (*result).qstride[2]=1;// (*ref).qstride[2];
}


#endif


#define CAL_XPHI_POINTERS_IN(number, type, value)   \
        HitXTile value;  hit_xtile(&value, &_value##number##_t, data_tile##number); \

#define CAL_XPHI_POINTERS_OUT(number, type, value)   \
        HitXTile value;  hit_xtile(&value, &_value##number##_t, data_tile##number); \


#define CAL_XPHI_POINTERS_IO(number, type, value)   \
        HitXTile value;  hit_xtile(&value, &_value##number##_t, data_tile##number); \

#define CAL_XPHI_POINTERS_IVAL(number, type, value)   \

#define CAL_XPHI_POINTER(number, iokind, type, value) \
                CAL_XPHI_POINTERS_##iokind(number, type, value)

#define CAL_XPHI_POINTERS1(io1, type1, value1) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS2(io1, type1, value1, io2, type2, value2) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)
#define CAL_XPHI_POINTERS3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)
#define CAL_XPHI_POINTERS4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)
#define CAL_XPHI_POINTERS5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)
#define CAL_XPHI_POINTERS6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)
#define CAL_XPHI_POINTERS7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS9(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9) \
                CAL_XPHI_POINTER(9, io9, type9, value9) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS10(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10) \
                CAL_XPHI_POINTER(10, io10, type10, value10) \
                CAL_XPHI_POINTER(9, io9, type9, value9) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS11(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11) \
                CAL_XPHI_POINTER(11, io11, type11, value11) \
                CAL_XPHI_POINTER(10, io10, type10, value10) \
                CAL_XPHI_POINTER(9, io9, type9, value9) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS12(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12) \
                CAL_XPHI_POINTER(12, io12, type12, value12) \
                CAL_XPHI_POINTER(11, io11, type11, value11) \
                CAL_XPHI_POINTER(10, io10, type10, value10) \
                CAL_XPHI_POINTER(9, io9, type9, value9) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)

#define CAL_XPHI_POINTERS13(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12, io13, type13, value13) \
                CAL_XPHI_POINTER(13, io13, type13, value13) \
                CAL_XPHI_POINTER(12, io12, type12, value12) \
                CAL_XPHI_POINTER(11, io11, type11, value11) \
                CAL_XPHI_POINTER(10, io10, type10, value10) \
                CAL_XPHI_POINTER(9, io9, type9, value9) \
                CAL_XPHI_POINTER(8, io8, type8, value8) \
                CAL_XPHI_POINTER(7, io7, type7, value7) \
                CAL_XPHI_POINTER(6, io6, type6, value6) \
                CAL_XPHI_POINTER(5, io5, type5, value5) \
                CAL_XPHI_POINTER(4, io4, type4, value4) \
                CAL_XPHI_POINTER(3, io3, type3, value3) \
                CAL_XPHI_POINTER(2, io2, type2, value2) \
                CAL_XPHI_POINTER(1, io1, type1, value1)




/*
#define CAL_XPHI_POINTERS1(io1, type1, value1)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \

#define CAL_XPHI_POINTERS2(io1, type1, value1, io2, type2, value2)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \

#define CAL_XPHI_POINTERS3(io1, type1, value1, io2, type2, value2, io3, type3, value3)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \

#define CAL_XPHI_POINTERS4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \
	HitXTile value4;  hit_xtile(&value4, &value4_t, data_tile4); \

#define CAL_XPHI_POINTERS5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \
	HitXTile value4;  hit_xtile(&value4, &value4_t, data_tile4); \
	HitXTile value5;  hit_xtile(&value5, &value5_t, data_tile5); \


#define CAL_XPHI_POINTERS6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \
	HitXTile value4;  hit_xtile(&value4, &value4_t, data_tile4); \
	HitXTile value5;  hit_xtile(&value5, &value5_t, data_tile5); \
	HitXTile value6;  hit_xtile(&value6, &value6_t, data_tile6); \


#define CAL_XPHI_POINTERS7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \
	HitXTile value4;  hit_xtile(&value4, &value4_t, data_tile4); \
	HitXTile value5;  hit_xtile(&value5, &value5_t, data_tile5); \
	HitXTile value6;  hit_xtile(&value6, &value6_t, data_tile6); \
	HitXTile value7;  hit_xtile(&value7, &value7_t, data_tile7); \

#define CAL_XPHI_POINTERS8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8)  \
	HitXTile value1;  hit_xtile(&value1, &value1_t, data_tile1); \
	HitXTile value2;  hit_xtile(&value2, &value2_t, data_tile2); \
	HitXTile value3;  hit_xtile(&value3, &value3_t, data_tile3); \
	HitXTile value4;  hit_xtile(&value4, &value4_t, data_tile4); \
	HitXTile value5;  hit_xtile(&value5, &value5_t, data_tile5); \
	HitXTile value6;  hit_xtile(&value6, &value6_t, data_tile6); \
	HitXTile value7;  hit_xtile(&value7, &value7_t, data_tile7); \
	HitXTile value8;  hit_xtile(&value8, &value8_t, data_tile8); \

*/
/*
#define CAL_XPHI_LAUNCH_CAST_IN(number, type, value)   \
	type value_p##number = (type)args[number + 1 ]; \
        HitTile value_t##number = *(HitTile*)(value_p##number) ;  \
	float *data_tile##number=(float*) (value_t##number).data;

#define CAL_XPHI_LAUNCH_CAST_OUT(number, type, value)   \
	type value_p##number = (type)args[number + 1 ]; \
        HitTile value_t##number = *(HitTile*)value_p##number ;  \
	float *data_tile##number=(float*) (value_t##number).data;
#define CAL_XPHI_LAUNCH_CAST_IO(number, type, value)   \
	type value_p##number = (type)args[number + 1 ]; \
        HitTile value_t##number = *(HitTile*)value_p##number ;  \
	float *data_tile##number=(float*) (value_t##number).data;

#define CAL_XPHI_LAUNCH_CAST_IVAL(number, type, value)   \
	type * value##number_p = (type *)args[number + 1 ]; \
        type value =* value##number_p; \
	
#define CAL_XPHI_LAUNCH_CAST(number, iokind, type, value) \
                CAL_XPHI_LAUNCH_CAST_##iokind(number, type, value)

 
#define CAL_XPHI_LAUNCH_CAST1(io1, type1, value1) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST2(io1, type1, value1, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(4, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5) \
                CAL_XPHI_LAUNCH_CAST(5, io5, type5, value5) \
                CAL_XPHI_LAUNCH_CAST(4, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6) \
                CAL_XPHI_LAUNCH_CAST(6, io6, type6, value6) \
                CAL_XPHI_LAUNCH_CAST(5, io5, type5, value5) \
                CAL_XPHI_LAUNCH_CAST(4, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
#define CAL_XPHI_LAUNCH_CAST7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7) \
                CAL_XPHI_LAUNCH_CAST(7, io7, type7, value7) \
                CAL_XPHI_LAUNCH_CAST(6, io6, type6, value6) \
                CAL_XPHI_LAUNCH_CAST(5, io5, type5, value5) \
                CAL_XPHI_LAUNCH_CAST(4, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)

#define CAL_XPHI_LAUNCH_CAST8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8) \
                CAL_XPHI_LAUNCH_CAST(8, io8, type8, value8) \
                CAL_XPHI_LAUNCH_CAST(7, io7, type7, value7) \
                CAL_XPHI_LAUNCH_CAST(6, io6, type6, value6) \
                CAL_XPHI_LAUNCH_CAST(5, io5, type5, value5) \
                CAL_XPHI_LAUNCH_CAST(4, io4, type4, value4) \
                CAL_XPHI_LAUNCH_CAST(3, io3, type3, value3) \
                CAL_XPHI_LAUNCH_CAST(2, io2, type2, value2) \
                CAL_XPHI_LAUNCH_CAST(1, io1, type1, value1)
*/

#define CAL_XPHI_WRAPPER_CAST_IN(number, type, value)   \
	type value##number## _p = (type)args[number + 1]; \
        HitTile _value##number## _t = *(HitTile*)value##number## _p;  float *data_tile##number=(float*) (_value##number## _t).data; \
	/*type value = (type)args[number + 1];*/ \
        /*HitTile _##value## _t = *(HitTile*)value; float *data_tile##number=(type*) (_##value## _t).data;*/ \

#define CAL_XPHI_WRAPPER_CAST_OUT(number, type, value)   \
	type value##number## _p = (type)args[number + 1]; \
        HitTile _value##number## _t = *(HitTile*)value##number## _p;  float *data_tile##number=(float*) (_value##number## _t).data; \
	/*type value = (type)args[number + 1];*/ \
        /*HitTile _##value## _t = *(HitTile*)value; float *data_tile##number=(type*) (_##value## _t).data;*/ \

#define CAL_XPHI_WRAPPER_CAST_IO(number, type, value)   \
	type value##number## _p = (type)args[number + 1]; \
        HitTile _value##number## _t = *(HitTile*)value##number## _p;  float *data_tile##number=(float*) (_value##number## _t).data; \
	/*type value = (type)args[number + 1];*/ \
        /*HitTile _##value## _t = *(HitTile*)value; float *data_tile##number=(type*) (_##value## _t).data;*/ \

#define CAL_XPHI_WRAPPER_CAST_IVAL(number, type, value)   \
        type value = *((type *)args[number + 1]); \

#define CAL_XPHI_WRAPPER_CAST(number, iokind, type, value) \
	CAL_XPHI_WRAPPER_CAST_##iokind(number, type, value)

/*
 * @eduardo 24-05-2017: Macros defined by Ana to cast the parameters to the kernel from the void** parameter of the wrapper.
 * 	Replaced by others.
#define CAL_XPHI_WRAPPER_CAST1(io1, type1, value1) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1=(float*) (_value1_t).data; \

#define CAL_XPHI_WRAPPER_CAST2(io1, type1, value1, io2, type2, value2) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1=(float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2=(float*) (_value2_t).data; \

#define CAL_XPHI_WRAPPER_CAST3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1=(float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2=(float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4];\
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3=(float*) (_value3_t).data; \

#define CAL_XPHI_WRAPPER_CAST4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*)(_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*)(_value4_t).data; \


#define CAL_XPHI_WRAPPER_CAST5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \

#define CAL_XPHI_WRAPPER_CAST6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \

#define CAL_XPHI_WRAPPER_CAST7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*)(_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*)(_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \


#define CAL_XPHI_WRAPPER_CAST8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \

#define CAL_XPHI_WRAPPER_CAST9(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \
	type9 value9_p = (type9)args[10]; \
	HitTile _value9_t = *(HitTile*)value9_p;  float *data_tile9= (float*) (_value9_t).data; \

#define CAL_XPHI_WRAPPER_CAST10(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \
	type9 value9_p = (type9)args[10]; \
	HitTile _value9_t = *(HitTile*)value9_p;  float *data_tile9= (float*) (_value9_t).data; \
	type10 value10_p = (type10)args[11]; \
	HitTile _value10_t = *(HitTile*)value10_p;  float *data_tile10= (float*) (_value10_t).data; \

#define CAL_XPHI_WRAPPER_CAST11(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \
	type9 value9_p = (type9)args[10]; \
	HitTile _value9_t = *(HitTile*)value9_p;  float *data_tile9= (float*) (_value9_t).data; \
	type10 value10_p = (type10)args[11]; \
	HitTile _value10_t = *(HitTile*)value10_p;  float *data_tile10= (float*) (_value10_t).data; \
	type11 value11_p = (type11)args[12]; \
	HitTile _value11_t = *(HitTile*)value11_p;  float *data_tile11= (float*) (_value11_t).data; \

#define CAL_XPHI_WRAPPER_CAST12(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \
	type9 value9_p = (type9)args[10]; \
	HitTile _value9_t = *(HitTile*)value9_p;  float *data_tile9= (float*) (_value9_t).data; \
	type10 value10_p = (type10)args[11]; \
	HitTile _value10_t = *(HitTile*)value10_p;  float *data_tile10= (float*) (_value10_t).data; \
	type11 value11_p = (type11)args[12]; \
	HitTile _value11_t = *(HitTile*)value11_p;  float *data_tile11= (float*) (_value11_t).data; \
	type12 value12_p = (type12)args[13]; \
	HitTile _value12_t = *(HitTile*)value12_p;  float *data_tile12= (float*) (_value12_t).data; \

#define CAL_XPHI_WRAPPER_CAST13(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12, io13, type13, value13 ) \
	type1 value1_p = (type1)args[2]; \
	HitTile _value1_t = *(HitTile*)value1_p;  float *data_tile1= (float*) (_value1_t).data; \
	type2 value2_p = (type2)args[3]; \
	HitTile _value2_t = *(HitTile*)value2_p;  float *data_tile2= (float*) (_value2_t).data; \
	type3 value3_p = (type3)args[4]; \
	HitTile _value3_t = *(HitTile*)value3_p;  float *data_tile3= (float*) (_value3_t).data; \
	type4 value4_p = (type4)args[5]; \
	HitTile _value4_t = *(HitTile*)value4_p;  float *data_tile4= (float*) (_value4_t).data; \
	type5 value5_p = (type5)args[6]; \
	HitTile _value5_t = *(HitTile*)value5_p;  float *data_tile5= (float*) (_value5_t).data; \
	type6 value6_p = (type6)args[7]; \
	HitTile _value6_t = *(HitTile*)value6_p;  float *data_tile6= (float*) (_value6_t).data; \
	type7 value7_p = (type7)args[8]; \
	HitTile _value7_t = *(HitTile*)value7_p;  float *data_tile7= (float*) (_value7_t).data; \
	type8 value8_p = (type8)args[9]; \
	HitTile _value8_t = *(HitTile*)value8_p;  float *data_tile8= (float*) (_value8_t).data; \
	type9 value9_p = (type9)args[10]; \
	HitTile _value9_t = *(HitTile*)value9_p;  float *data_tile9= (float*) (_value9_t).data; \
	type10 value10_p = (type10)args[11]; \
	HitTile _value10_t = *(HitTile*)value10_p;  float *data_tile10= (float*) (_value10_t).data; \
	type11 value11_p = (type11)args[12]; \
	HitTile _value11_t = *(HitTile*)value11_p;  float *data_tile11= (float*) (_value11_t).data; \
	type12 value12_p = (type12)args[13]; \
	HitTile _value12_t = *(HitTile*)value12_p;  float *data_tile12= (float*) (_value12_t).data; \
	type13 value13_p = (type13)args[14]; \
	HitTile _value13_t = *(HitTile*)value13_p;  float *data_tile13= (float*) (_value13_t).data; \
*/

#define CAL_XPHI_WRAPPER_CAST1(io1, type1, value1) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST2(io1, type1, value1, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)
#define CAL_XPHI_WRAPPER_CAST7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST9(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(9, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST10(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10 ) \
                CAL_XPHI_WRAPPER_CAST(10, io10, type10, value10) \
                CAL_XPHI_WRAPPER_CAST(9, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST11(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11 ) \
                CAL_XPHI_WRAPPER_CAST(11, io11, type11, value11) \
                CAL_XPHI_WRAPPER_CAST(10, io10, type10, value10) \
                CAL_XPHI_WRAPPER_CAST(9, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST12(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12 ) \
                CAL_XPHI_WRAPPER_CAST(12, io12, type12, value12) \
                CAL_XPHI_WRAPPER_CAST(11, io11, type11, value11) \
                CAL_XPHI_WRAPPER_CAST(10, io10, type10, value10) \
                CAL_XPHI_WRAPPER_CAST(9, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

#define CAL_XPHI_WRAPPER_CAST13(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12, io13, type13, value13 ) \
                CAL_XPHI_WRAPPER_CAST(13, io13, type13, value13) \
                CAL_XPHI_WRAPPER_CAST(12, io12, type12, value12) \
                CAL_XPHI_WRAPPER_CAST(11, io11, type11, value11) \
                CAL_XPHI_WRAPPER_CAST(10, io10, type10, value10) \
                CAL_XPHI_WRAPPER_CAST(9, io9, type9, value9) \
                CAL_XPHI_WRAPPER_CAST(8, io8, type8, value8) \
                CAL_XPHI_WRAPPER_CAST(7, io7, type7, value7) \
                CAL_XPHI_WRAPPER_CAST(6, io6, type6, value6) \
                CAL_XPHI_WRAPPER_CAST(5, io5, type5, value5) \
                CAL_XPHI_WRAPPER_CAST(4, io4, type4, value4) \
                CAL_XPHI_WRAPPER_CAST(3, io3, type3, value3) \
                CAL_XPHI_WRAPPER_CAST(2, io2, type2, value2) \
                CAL_XPHI_WRAPPER_CAST(1, io1, type1, value1)

//Wrapper Offload
//
#define STRINGIFY(a) #a



#define CAL_XPHI_OFFLOAD_IN(number, type, value)   \
	in(_value##number##_t) in(data_tile##number:length(0) alloc_if(0) free_if(0)) 	

#define CAL_XPHI_OFFLOAD_IO(number, type, value)   \
	in(_value##number##_t) in(data_tile##number:length(0) alloc_if(0) free_if(0)) 	

#define CAL_XPHI_OFFLOAD_OUT(number, type, value)   \
	in(_value##number##_t) in(data_tile##number:length(0) alloc_if(0) free_if(0)) 	

#define CAL_XPHI_OFFLOAD_IVAL(number, type, value)   \


#define CAL_XPHI_OFFLOAD(number, iokind, type, value) \
                CAL_XPHI_OFFLOAD_##iokind(number, type, value)


#define CAL_XPHI_OFFLOAD_PARAMS1(io1, type1, value1) \
                offload target(mic:0) in(num_phis) in(threads:length(3)) CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS2(io1, type1, value1, io2, type2, value2) \
         	offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS9(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(9, io9, type9, value9) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS10(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(10, io10, type10, value10) \
                CAL_XPHI_OFFLOAD(9, io9, type9, value9) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS11(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(11, io11, type11, value11) \
                CAL_XPHI_OFFLOAD(10, io10, type10, value10) \
                CAL_XPHI_OFFLOAD(9, io9, type9, value9) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS12(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(12, io12, type12, value12) \
                CAL_XPHI_OFFLOAD(11, io11, type11, value11) \
                CAL_XPHI_OFFLOAD(10, io10, type10, value10) \
                CAL_XPHI_OFFLOAD(9, io9, type9, value9) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)

#define CAL_XPHI_OFFLOAD_PARAMS13(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12, io13, type13, value13) \
		offload target(mic:0) in(num_phis) in(threads:length(3)) \
                CAL_XPHI_OFFLOAD(13, io13, type13, value13) \
                CAL_XPHI_OFFLOAD(12, io12, type12, value12) \
                CAL_XPHI_OFFLOAD(11, io11, type11, value11) \
                CAL_XPHI_OFFLOAD(10, io10, type10, value10) \
                CAL_XPHI_OFFLOAD(9, io9, type9, value9) \
                CAL_XPHI_OFFLOAD(8, io8, type8, value8) \
                CAL_XPHI_OFFLOAD(7, io7, type7, value7) \
                CAL_XPHI_OFFLOAD(6, io6, type6, value6) \
                CAL_XPHI_OFFLOAD(5, io5, type5, value5) \
                CAL_XPHI_OFFLOAD(4, io4, type4, value4) \
                CAL_XPHI_OFFLOAD(3, io3, type3, value3) \
                CAL_XPHI_OFFLOAD(2, io2, type2, value2) \
                CAL_XPHI_OFFLOAD(1, io1, type1, value1)


/*
#define CAL_XPHI_OFFLOAD_PARAMS1(io1, type1, value1) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(data_tile1:length(0) alloc_if(0) free_if(0))

#define CAL_XPHI_OFFLOAD_PARAMS2(io1, type1, value1, io2, type2, value2) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0))

#define CAL_XPHI_OFFLOAD_PARAMS3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(_value1_t) in(_value2_t) in(_value3_t) in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0))

#define CAL_XPHI_OFFLOAD_PARAMS4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(value3_t) in(value4_t) in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0)) in(data_tile4:length(0) alloc_if(0) free_if(0))


#define CAL_XPHI_OFFLOAD_PARAMS5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5 ) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(value3_t) in(value4_t) in(value5_t) in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0)) in(data_tile4:length(0) alloc_if(0) free_if(0)) in(data_tile5:length(0) alloc_if(0) free_if(0))

#define CAL_XPHI_OFFLOAD_PARAMS6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6 ) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(value3_t) in(value4_t) in(value5_t) in(value6_t)  in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0)) in(data_tile4:length(0) alloc_if(0) free_if(0)) in(data_tile5:length(0) alloc_if(0) free_if(0)) in(data_tile6:length(0) alloc_if(0) free_if(0)) 


#define CAL_XPHI_OFFLOAD_PARAMS7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7 ) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(value3_t) in(value4_t) in(value5_t) in(value6_t) in(value7_t)  in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0)) in(data_tile4:length(0) alloc_if(0) free_if(0)) in(data_tile5:length(0) alloc_if(0) free_if(0)) in(data_tile6:length(0) alloc_if(0) free_if(0)) in(data_tile7:length(0) alloc_if(0) free_if(0))

#define CAL_XPHI_OFFLOAD_PARAMS8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8 ) \
	offload target(mic:0) in(num_phis) in(threads:length(3)) in(value1_t) in(value2_t) in(value3_t) in(value4_t) in(value5_t) in(value6_t) in(value7_t) in(value8_t)  in(data_tile1:length(0) alloc_if(0) free_if(0)) in(data_tile2:length(0) alloc_if(0) free_if(0)) in(data_tile3:length(0) alloc_if(0) free_if(0)) in(data_tile4:length(0) alloc_if(0) free_if(0)) in(data_tile5:length(0) alloc_if(0) free_if(0)) in(data_tile6:length(0) alloc_if(0) free_if(0)) in(data_tile7:length(0) alloc_if(0) free_if(0))  in(data_tile8:length(0) alloc_if(0) free_if(0))


*/


// Wrappers for the param values

#define CAL_XPHI_WRAPPER_VALUES0(NULL) ;

#define CAL_XPHI_WRAPPER_VALUES1(io1, type1, value1) \
	value1

#define CAL_XPHI_WRAPPER_VALUES2(io1, type1, value1, io2, type2, value2) \
	value1, value2

#define CAL_XPHI_WRAPPER_VALUES3(io1, type1, value1, io2, type2, value2, io3, type3, value3) \
	value1, value2, value3

#define CAL_XPHI_WRAPPER_VALUES4(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4) \
	value1, value2, value3, value4

#define CAL_XPHI_WRAPPER_VALUES5(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5 ) \
	value1, value2, value3, value4, value5

#define CAL_XPHI_WRAPPER_VALUES6(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6 ) \
	value1, value2, value3, value4, value5, value6

#define CAL_XPHI_WRAPPER_VALUES7(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7 ) \
	value1, value2, value3, value4, value5, value6, value7

#define CAL_XPHI_WRAPPER_VALUES8(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8 ) \
	value1, value2, value3, value4, value5, value6, value7, value8

#define CAL_XPHI_WRAPPER_VALUES9(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9 ) \
	value1, value2, value3, value4, value5, value6, value7, value8, value9

#define CAL_XPHI_WRAPPER_VALUES10(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10 ) \
	value1, value2, value3, value4, value5, value6, value7, value8, value9, value10

#define CAL_XPHI_WRAPPER_VALUES11(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11  ) \
	value1, value2, value3, value4, value5, value6, value7, value8, value9, value10, value11

#define CAL_XPHI_WRAPPER_VALUES12(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12 ) \
	value1, value2, value3, value4, value5, value6, value7, value8, value9, value10, value11, value12

#define CAL_XPHI_WRAPPER_VALUES13(io1, type1, value1, io2, type2, value2, io3, type3, value3, io4, type4, value4, io5, type5, value5, io6, type6, value6, io7, type7, value7, io8, type8, value8, io9, type9, value9, io10, type10, value10, io11, type11, value11, io12, type12, value12, io13, type13, value13 ) \
	value1, value2, value3, value4, value5, value6, value7, value8, value9, value10, value11, value12, value13


// Wrapper for kernel launches
#undef CAL_XPHI_LAUNCH0
#define CAL_XPHI_LAUNCH0(name, comm, threads) name##_xphi(&comm.impl.xphi, threads); 
#undef CAL_XPHI_LAUNCH1
#define CAL_XPHI_LAUNCH1(name, comm, threads, param1) name##_xphi(&comm.impl.xphi, threads, param1); 
#undef CAL_XPHI_LAUNCH2
#define CAL_XPHI_LAUNCH2(name, comm, threads, param1, param2) name##_xphi(&comm.impl.xphi, threads, param1, param2); 
#undef CAL_XPHI_LAUNCH3
#define CAL_XPHI_LAUNCH3(name, comm, threads, param1, param2, param3) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3); 
#undef CAL_XPHI_LAUNCH4
#define CAL_XPHI_LAUNCH4(name, comm, threads, param1, param2, param3, param4) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4);
#undef CAL_XPHI_LAUNCH5
#define CAL_XPHI_LAUNCH5(name, comm, threads, param1, param2, param3, param4, param5) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5);
#undef CAL_XPHI_LAUNCH6
#define CAL_XPHI_LAUNCH6(name, comm, threads, param1, param2, param3, param4, param5, param6) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6);
#undef CAL_XPHI_LAUNCH7
#define CAL_XPHI_LAUNCH7(name, comm, threads, param1, param2, param3, param4, param5, param6, param7) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7);
#undef CAL_XPHI_LAUNCH8
#define CAL_XPHI_LAUNCH8(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8);
#undef CAL_XPHI_LAUNCH9
#define CAL_XPHI_LAUNCH9(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9);
#undef CAL_XPHI_LAUNCH10
#define CAL_XPHI_LAUNCH10(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);
#undef CAL_XPHI_LAUNCH11
#define CAL_XPHI_LAUNCH11(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11);
#undef CAL_XPHI_LAUNCH12
#define CAL_XPHI_LAUNCH12(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12);
#undef CAL_XPHI_LAUNCH13
#define CAL_XPHI_LAUNCH13(name, comm, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13) name##_xphi(&comm.impl.xphi, threads, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13);



// Kernel call

#define MIN(a,b) (((a)<(b))?(a):(b))

#define CAL_KERNEL_XPHI(name, nparams, params...) \
	static void __attribute__((target(mic))) kernel_xphi_##name(CALThread threadId, CAL_XPHI_KWRAPPER_PARAMS##nparams(params)); \
	static inline void wrapper_xphi_##name(int ind, void** args){ \
		CALCntrlXPHI* cntrl = (CALCntrlXPHI*) args[0]; \
		CALThread* threads = (CALThread*)args[1]; \
		int num_phis=cntrl->XPHIS ;\
		CAL_XPHI_WRAPPER_CAST##nparams(params); \
		/*CAL_XPHI_WRAPPER_CAST##nparams(params);*/ \
                _Pragma( STRINGIFY(CAL_XPHI_OFFLOAD_PARAMS##nparams(params)) ) \
                _Pragma("omp parallel") \
                { \
		double Clock = omp_get_wtime() ;\
                CAL_XPHI_POINTERS##nparams(params); \
                { \
                int i,j,k; \
                CALThread threadId; \
                if (threads->z > 1){ \
                        _Pragma("omp for private(i,j,k)") \
                         for(i=0; i<threads->x; i++){  \
                                 for(j=0; j<threads->y; j++){           \
                                        for(k=0; k<threads->z; k++){            \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = k; \
                                                kernel_xphi_##name(threadId, CAL_XPHI_WRAPPER_VALUES##nparams(params));  \
                        } } }\
                } \
                else if (threads->y > 1){\
                        _Pragma("omp for private(i,j)") \
                         for(i=0; i<threads->x; i++){  \
                                 for(j=0; j<threads->y; j++){           \
                                                threadId.x = i; \
                                                threadId.y = j; \
                                                threadId.z = 0; \
                                                kernel_xphi_##name(threadId, CAL_XPHI_WRAPPER_VALUES##nparams(params));  \
                        } } \
                }\
                else {\
                        _Pragma("omp for private(i)") \
                         for(i=0; i<threads->x; i++){  \
                                                threadId.x = i; \
                                                threadId.y = 0; \
                                                threadId.z = 0; \
                                                kernel_xphi_##name(threadId, CAL_XPHI_WRAPPER_VALUES##nparams(params));  \
                        }  \
                }\
		Clock = omp_get_wtime() - Clock; \
		} \
               } \
	        \
	} \
	void name##_xphi(CALCntrlXPHI* cntrl, CALThread thread, CAL_XPHI_WRAPPER_PARAMS##nparams(params)) { \
                        CAL_CntrlXPHIAddTask(cntrl, wrapper_xphi_##name, thread, nparams, CAL_XPHI_WRAPPER_VALUES##nparams(params));		\
		\
	} \
	void name##_gpu(CALCntrlGPU* cntrl, CALThread thread, CAL_GPU_WRAPPER_PARAMS##nparams(params)) { \
		\
	} \
	void name##_cpu(CALCntrlCPU* cntrl, CALThread thread, CAL_XPHI_WRAPPER_PARAMS##nparams(params)) { \
		\
	} \
	static void __attribute__((target(mic))) kernel_xphi_##name(CALThread threadId, CAL_XPHI_KWRAPPER_PARAMS##nparams(params))






