#ifdef __ICC

#define STRINGIFY(a) #a

/* Macros to cast the data structures for offloading */
#define CAST_I_IN(type, name)		float *t_##name = name.data; int *orig_##name; orig_##name = (int*)(name.origAcumCard);
#define CAST_I_OUT(type, name)		float *t_##name = name.data; int *orig_##name; orig_##name = (int*)(name.origAcumCard);
#define CAST_I_IO(type, name)		float *t_##name = name.data; int *orig_##name; orig_##name = (int*)(name.origAcumCard);
#define CAST_I_INVAL(type, name)		

#define CAL_CAST_IN(role, type, name)			CAST_I_##role(type,name)		

#define CAL_PROTO_CAST_IN( list, numArgs, ... )       CAL_PROTO_CAST_IN_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_1( role, type, name )       CAL_CAST_IN(role, type, name)
#define CAL_PROTO_CAST_IN_2( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_1( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_3( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_2( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_4( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_3( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_5( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_4( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_6( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_5( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_7( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_6( __VA_ARGS__ )
#define CAL_PROTO_CAST_IN_8( role, type, name, ... )  CAL_CAST_IN(role, type, name) CAL_PROTO_CAST_IN_7( __VA_ARGS__ )

/* Macros to cast the data structures for offloading */
#define CAST_O_IN(type, name)		name.data = t_##name; name.origAcumCard[0] = orig_##name[0]; name.origAcumCard[1] = orig_##name[1]; name.origAcumCard[2] = orig_##name[2]; 
#define CAST_O_OUT(type, name)		name.data = t_##name; name.origAcumCard[0] = orig_##name[0]; name.origAcumCard[1] = orig_##name[1]; name.origAcumCard[2] = orig_##name[2];
#define CAST_O_IO(type, name)		name.data = t_##name; name.origAcumCard[0] = orig_##name[0]; name.origAcumCard[1] = orig_##name[1]; name.origAcumCard[2] = orig_##name[2];
#define CAST_O_INVAL(type, name)		

#define CAL_CAST_OUT(role, type, name)			CAST_O_##role(type,name)		

#define CAL_PROTO_CAST_OUT( list, numArgs, ... )       CAL_PROTO_CAST_OUT_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_1( role, type, name )       CAL_CAST_OUT(role, type, name)
#define CAL_PROTO_CAST_OUT_2( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_1( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_3( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_2( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_4( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_3( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_5( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_4( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_6( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_5( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_7( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_6( __VA_ARGS__ )
#define CAL_PROTO_CAST_OUT_8( role, type, name, ... )  CAL_CAST_OUT(role, type, name) CAL_PROTO_CAST_OUT_7( __VA_ARGS__ )

/* Macros needed to perform the offloading in the Xeon Phi */
#define KTILE_OFFLOAD_INVAL(type, name)             in(name) 
#define KTILE_OFFLOAD_IN(type, name)                in(name) in(orig_##name:length(3)) in(t_##name:length(0) alloc_if(0) free_if(0))
#define KTILE_OFFLOAD_IO(type, name)                in(name) in(orig_##name:length(3)) in(t_##name:length(0) alloc_if(0) free_if(0))
#define KTILE_OFFLOAD_OUT(type, name)               in(name) in(orig_##name:length(3)) in(t_##name:length(0) alloc_if(0) free_if(0))

#define CAL_OFFLOAD_KTILE(role,type, name)          KTILE_OFFLOAD_##role(type,name) 

#define CAL_PROTO_OFFLOAD_KTILE_1( role, type, name )       CAL_OFFLOAD_KTILE(role, type, name)
#define CAL_PROTO_OFFLOAD_KTILE_2( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_1( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_3( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_2( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_4( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_3( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_5( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_4( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_6( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_5( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_7( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_6( __VA_ARGS__ )
#define CAL_PROTO_OFFLOAD_KTILE_8( role, type, name, ... )  CAL_OFFLOAD_KTILE(role, type, name) CAL_PROTO_OFFLOAD_KTILE_7( __VA_ARGS__ )


#define CAL_PROTO_OFFLOAD_KTILE_R( list, numArgs, ... )        STRINGIFY(offload target(mic:0) in(threads) __VA_ARGS__)
#define CAL_PROTO_OFFLOAD_KTILE( list, numArgs, ... )        CAL_PROTO_OFFLOAD_KTILE_R( list, numArgs, CAL_PROTO_OFFLOAD_KTILE_##numArgs( __VA_ARGS__ ) )

#else

#endif
