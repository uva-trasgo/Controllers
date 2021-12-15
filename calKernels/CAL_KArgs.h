/*
 * CAL_KArgs: Macros to process lists of arguments in kernels definitions and launches
 *
 * @version 1.1
 * @author Ana Moreton-Fernandez
 * @date Oct 2017
 */

#ifndef _CAL_KArgs_
#define _CAL_KArgs_

/*
 *******************************************************************************************
 *******************************************************************************************
 *************************************  Basics  ********************************************
 *******************************************************************************************
 *******************************************************************************************
 */

/* Copy parameter types and names */
#define CAL_PROTO_TYPED( list, numArgs, ... )   CAL_PROTO_TYPED_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_TYPED_1( role, type, name )           type name
#define CAL_PROTO_TYPED_2( role, type, name, ... )      type name, CAL_PROTO_TYPED_1( __VA_ARGS__ )
#define CAL_PROTO_TYPED_3( role, type, name, ... )      type name, CAL_PROTO_TYPED_2( __VA_ARGS__ )
#define CAL_PROTO_TYPED_4( role, type, name, ... )      type name, CAL_PROTO_TYPED_3( __VA_ARGS__ )
#define CAL_PROTO_TYPED_5( role, type, name, ... )      type name, CAL_PROTO_TYPED_4( __VA_ARGS__ )
#define CAL_PROTO_TYPED_6( role, type, name, ... )      type name, CAL_PROTO_TYPED_5( __VA_ARGS__ )
#define CAL_PROTO_TYPED_7( role, type, name, ... )      type name, CAL_PROTO_TYPED_6( __VA_ARGS__ )
#define CAL_PROTO_TYPED_8( role, type, name, ... )      type name, CAL_PROTO_TYPED_7( __VA_ARGS__ )
#define CAL_PROTO_TYPED_9( role, type, name, ... )      type name, CAL_PROTO_TYPED_8( __VA_ARGS__ )

/* Copy pointer parameter types and names */
#define CAL_PROTO_TYPED_POINTER( list, numArgs, ... )   CAL_PROTO_TYPED_POINTER_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_1( role, type, name )           type* name
#define CAL_PROTO_TYPED_POINTER_2( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_1( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_3( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_2( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_4( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_3( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_5( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_4( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_6( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_5( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_7( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_6( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_8( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_7( __VA_ARGS__ )
#define CAL_PROTO_TYPED_POINTER_9( role, type, name, ... )      type* name, CAL_PROTO_TYPED_POINTER_8( __VA_ARGS__ )

/* 
 * Change list of arguments to list of pointers
 */
// #define CAL_ARGS_TO_POINTERS( ... ) CAL_ARGS_TO_POINTERS_VARIADIC( CAL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)
// #define CAL_ARGS_TO_POINTERS_VARIADIC() CAL_ARGS_TO_POINTERS_##num_args( __VA_ARGS__)
#define CAL_ARGS_POINTERS_1( _1 )  &(_1)
#define CAL_ARGS_POINTERS_2( _1, _2 )  &(_1), &(_2) 
#define CAL_ARGS_POINTERS_3( _1, _2, _3)  &(_1), &(_2), &(_3)
#define CAL_ARGS_POINTERS_4( _1, _2, _3, _4)  &(_1), &(_2), &(_3), &(_4)
#define CAL_ARGS_POINTERS_5( _1, _2, _3, _4, _5)  &(_1), &(_2), &(_3), &(_4), &(_5)
#define CAL_ARGS_POINTERS_6( _1, _2, _3, _4, _5, _6)  &(_1), &(_2), &(_3), &(_4), &(_5), &(_6)
#define CAL_ARGS_POINTERS_7( _1, _2, _3, _4, _5, _6, _7)  &(_1), &(_2), &(_3), &(_4), &(_5), &(_6), &(_7)
#define CAL_ARGS_POINTERS_8( _1, _2, _3, _4, _5, _6, _7, _8)  &(_1), &(_2), &(_3), &(_4), &(_5), &(_6), &(_7), &(_8)
#define CAL_ARGS_POINTERS_9( _1, _2, _3, _4, _5, _6, _7, _8, _9)  &(_1), &(_2), &(_3), &(_4), &(_5), &(_6), &(_7), &(_8), &(_9)

/* 
 * Override CAL_ARGS_TO_POINTERS macro by number of variadic arguments 
 */
#define CAL_ARGS_POINTERS_N( _1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ... ) NAME
#define CAL_ARGS_TO_POINTERS( ... ) CAL_ARGS_POINTERS_N(__VA_ARGS__, \
			CAL_ARGS_POINTERS_9, \
			CAL_ARGS_POINTERS_8, \
			CAL_ARGS_POINTERS_7, \
			CAL_ARGS_POINTERS_6, \
			CAL_ARGS_POINTERS_5, \
			CAL_ARGS_POINTERS_4, \
			CAL_ARGS_POINTERS_3, \
			CAL_ARGS_POINTERS_2, \
			CAL_ARGS_POINTERS_1, \
		)(__VA_ARGS__)

/*
 * Arguments list size (sum of the type sizes of all the arguments)
 */
#define CAL_ARG_LIST_SIZE( numArgs, ... ) CAL_ARG_LIST_SIZE_##numArgs( __VA_ARGS__ )

#define CAL_ARG_LIST_SIZE_1( role, type, name ) 		\
		sizeof(type)
#define CAL_ARG_LIST_SIZE_2( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_1( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_3( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_2( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_4( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_3( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_5( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_4( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_6( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_5( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_7( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_6( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_8( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_7( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_9( role, type, name, ... ) 	\
		sizeof(type)+CAL_ARG_LIST_SIZE_8( __VA_ARGS__ )


/*
 * List store: Store a copy of the values in a contiguos buffer
 */
#define	CAL_ARG_LIST_STORE( list, numArgs, ... ) CAL_ARG_LIST_STORE_##numArgs( list, __VA_ARGS__ )

#define CAL_ARG_LIST_STORE_1( list, role, type, name ) 	\
		*((type *)(list)) = name
#define CAL_ARG_LIST_STORE_2( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_1( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_3( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_2( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_4( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_3( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_5( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_4( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_6( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_5( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_7( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_6( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_8( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_7( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_9( list, role, type, name, ... ) 	\
		*((type *)(list)) = name; CAL_ARG_LIST_STORE_8( (uint8_t*)list+sizeof(type), __VA_ARGS__ )

/*
 * Create list buffer: Create the buffer and invocates the store macro
 */
#define CAL_ARG_LIST_CREATE( listArgs, numArgs, ... ) \
		listArgs = malloc( CAL_ARG_LIST_SIZE( numArgs, __VA_ARGS__ ) ); \
		if (listArgs == NULL ) { fprintf(stderr, "CAL Internal error: Args Malloc\n"); exit(-1); } \
		CAL_ARG_LIST_STORE( listArgs, numArgs, __VA_ARGS__ )

/*
 * Destroy list buffer
 */
#define CAL_ARG_LIST_DESTROY( listArgs )	free( listArgs )

/*
 * Access to list buffer values: Macro to generate the code to access a specific list
 * 		of arguments types in a function call
 */
#define	CAL_ARG_LIST_ACCESS( list, numArgs, ... ) CAL_ARG_LIST_ACCESS_##numArgs( list, __VA_ARGS__ )

#define CAL_ARG_LIST_ACCESS_1( list, role, type, name )			\
		*((type *)(list))
#define CAL_ARG_LIST_ACCESS_2( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_1( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_3( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_2( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_4( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_3( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_5( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_4( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_6( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_5( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_7( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_6( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_8( list, role, type, name, ... )	\
		*((type *)(list)), CAL_ARG_LIST_ACCESS_7( (uint8_t*)list+sizeof(type), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_9( list, role, type, name, ... )	\


/*
 *******************************************************************************************
 *******************************************************************************************
 **************************************  KTILES  *******************************************
 *******************************************************************************************
 *******************************************************************************************
 */

/*
 * Macros used to allow the use of KTiles and the GPU Kernel charazterization
 */ 

/* @ana 28-07-2017 Adding GPU features */
#define CAL_GPU_KERNEL_STATIC 1
#define CAL_GPU_KERNEL_DYNAMIC 2

#define CAL_KERNEL_GPU_CHAR_STATIC(name, dimBlock, A, B, C) \
        int CAL_GPU_kchar_type_##name  = CAL_GPU_KERNEL_STATIC; \
dim3 CAL_GPU_kchar_block_##name[CAL_NARCHS] = CAL_GPU_kchar_##dimBlock##A##B##C; 

#define CAL_GPU_kchar_threads(name,arch) CAL_GPU_kchar_block_##name[arch]
#define CAL_GPU_kchar_grid(name,arch,threads) \
        dim3((threads.x + CAL_GPU_kchar_threads(name, arch).x - 1) / CAL_GPU_kchar_threads(name, arch).x, \
                        (threads.y + CAL_GPU_kchar_threads(name, arch).y - 1) / CAL_GPU_kchar_threads(name, arch).y, \
                        (threads.z + CAL_GPU_kchar_threads(name, arch).z - 1) / CAL_GPU_kchar_threads(name, arch).z);



/* Copy parameter types and names for GPU*/
#define KTILE_TYPED_INVAL(type, name)             type name
#define KTILE_TYPED_IN(type, name)                K##type name
#define KTILE_TYPED_IO(type, name)                K##type name
#define KTILE_TYPED_OUT(type, name)               K##type name

#define CAL_TYPED_KTILE(role,type, name)          KTILE_TYPED_##role(type,name) 

#define CAL_PROTO_TYPED_KTILE( list, numArgs, ... )       CAL_PROTO_TYPED_KTILE_##numArgs( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_1( role, type, name )       CAL_TYPED_KTILE(role, type, name)
#define CAL_PROTO_TYPED_KTILE_2( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_1( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_3( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_2( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_4( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_3( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_5( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_4( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_6( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_5( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_7( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_6( __VA_ARGS__ )
#define CAL_PROTO_TYPED_KTILE_8( role, type, name, ... )  CAL_TYPED_KTILE(role, type, name), CAL_PROTO_TYPED_KTILE_7( __VA_ARGS__ )


/*
 * Arguments list size for GPU (sum of the type sizes of all the arguments)
 */
#define SIZE_KTILE_INVAL(type)	sizeof(type)
#define SIZE_KTILE_IN(type)	sizeof(K##type)
#define SIZE_KTILE_IO(type)	sizeof(K##type)
#define SIZE_KTILE_OUT(type)	sizeof(K##type)

#define LIST_SIZE_KTILE(role, type, name)		SIZE_KTILE_##role(type)

#define CAL_ARG_LIST_SIZE_KTILE( numArgs, ... ) CAL_ARG_LIST_SIZE_KTILE_##numArgs( __VA_ARGS__ )

#define CAL_ARG_LIST_SIZE_KTILE_1( role, type, name ) 		\
		LIST_SIZE_KTILE(role, type, name)
#define CAL_ARG_LIST_SIZE_KTILE_2( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_1( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_3( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_2( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_4( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_3( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_5( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_4( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_6( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_5( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_7( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_6( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_8( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_7( __VA_ARGS__ )
#define CAL_ARG_LIST_SIZE_KTILE_9( role, type, name, ... ) 	\
		LIST_SIZE_KTILE(role, type, name)+CAL_ARG_LIST_SIZE_KTILE_8( __VA_ARGS__ )

/*
 * Macros for casting the arguments for the GPU devices
 */
#define KTILE_STORE_INVAL(list,type,name)		\
		 *((type *)(list)) = (*name);

#define KTILE_STORE_IN(list,type,name)		\
		K##type k_##name;				\
		if (deviceType == dGPU){k_##name.data = name->devData;}	\
		if (deviceType == CPU){k_##name.data = name->data;}	\
		if (deviceType == XPhi){k_##name.data = name->data;}	\
		k_##name.origAcumCard[0] = name->origAcumCard[0];				\
		k_##name.origAcumCard[1] = name->origAcumCard[1];				\
		k_##name.origAcumCard[2] = name->origAcumCard[2];				\
		k_##name.card[0] = hit_tileDimCard( (*name), 0 );				\
		k_##name.card[1] = hit_tileDimCard( (*name), 1 );				\
		k_##name.card[2] = hit_tileDimCard( (*name), 2 );				\
		 *((K##type *)(list)) = k_##name;                                  

#define KTILE_STORE_IO(list,type,name)		\
		K##type k_##name;				\
		if (deviceType == dGPU){k_##name.data = name->devData;}	\
		if (deviceType == CPU){k_##name.data = name->data;}	\
		if (deviceType == XPhi){k_##name.data = name->data;}	\
		k_##name.origAcumCard[0] = name->origAcumCard[0];				\
		k_##name.origAcumCard[1] = name->origAcumCard[1];				\
		k_##name.origAcumCard[2] = name->origAcumCard[2];				\
		k_##name.card[0] = hit_tileDimCard( (*name), 0 );				\
		k_##name.card[1] = hit_tileDimCard( (*name), 1 );				\
		k_##name.card[2] = hit_tileDimCard( (*name), 2 );				\
		 *((K##type *)(list)) = k_##name;                                  

#define KTILE_STORE_OUT(list,type,name)		\
		K##type k_##name;				\
		if (deviceType == dGPU){k_##name.data = name->devData;}	\
		if (deviceType == CPU){k_##name.data = name->data;}	\
		if (deviceType == XPhi){k_##name.data = name->data;}	\
		k_##name.origAcumCard[0] = name->origAcumCard[0];				\
		k_##name.origAcumCard[1] = name->origAcumCard[1];				\
		k_##name.origAcumCard[2] = name->origAcumCard[2];				\
		k_##name.card[0] = hit_tileDimCard( (*name), 0 );				\
		k_##name.card[1] = hit_tileDimCard( (*name), 1 );				\
		k_##name.card[2] = hit_tileDimCard( (*name), 2 );				\
		 *((K##type *)(list)) = k_##name;                                  

/*
 * List store for Ktile types: Store a copy of the values in a contiguos buffer, also casting to kTiles and transferring memory
 */
#define KTILE_STORE(list, role, type, name)	KTILE_STORE_##role(list,type,name) 

#define	CAL_ARG_LIST_STORE_KTILE( list, numArgs, ... ) CAL_ARG_LIST_STORE_KTILE_##numArgs( list, __VA_ARGS__ )

#define CAL_ARG_LIST_STORE_KTILE_1( list, role, type, name ) 	\
		KTILE_STORE(list, role, type, name);
#define CAL_ARG_LIST_STORE_KTILE_2( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_1( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_3( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_2( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_4( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_3( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_5( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_4( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_6( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_5( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_7( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_6( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_8( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_7( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_STORE_KTILE_9( list, role, type, name, ... ) 	\
		KTILE_STORE(list, role, type, name); CAL_ARG_LIST_STORE_KTILE_8( (uint8_t*)list+LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )

/*
 * Create list buffer: Create the buffer and invocates the store macro
 */
#define CAL_ARG_LIST_CREATE_KTILE( listArgs, numArgs, ... ) \
		listArgs = malloc( CAL_ARG_LIST_SIZE_KTILE( numArgs, __VA_ARGS__ ) ); \
		if (listArgs == NULL ) { fprintf(stderr, "CAL Internal error: Args Malloc\n"); exit(-1); } \
		CAL_ARG_LIST_STORE_KTILE( listArgs, numArgs, __VA_ARGS__ )


/*
 * Access to list buffer values: Macro to generate the code to access a specific list
 * 		of arguments types in a function call
 */
#define KTILE_ACCESS_INVAL(list,type,name)         \
		 *((type *)(list))

#define KTILE_ACCESS_IN(list,type,name)         \
		 *((K##type *)(list))

#define KTILE_ACCESS_IO(list,type,name)         \
		 *((K##type *)(list))

#define KTILE_ACCESS_OUT(list,type,name)         \
		 *((K##type *)(list))

#define KTILE_ACCESS(list, role, type, name)       KTILE_ACCESS_##role(list,type,name) 

#define	CAL_ARG_LIST_ACCESS_KTILE( list, numArgs, ... ) CAL_ARG_LIST_ACCESS_KTILE_##numArgs( list, __VA_ARGS__ )

#define CAL_ARG_LIST_ACCESS_KTILE_1( list, role, type, name )			\
		KTILE_ACCESS(list, role, type, name) 
#define CAL_ARG_LIST_ACCESS_KTILE_2( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_1( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_3( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_2( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_4( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_3( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_5( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_4( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_6( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_5( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_7( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_6( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_8( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_7( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )
#define CAL_ARG_LIST_ACCESS_KTILE_9( list, role, type, name, ... )	\
		KTILE_ACCESS(list, role, type, name) , CAL_ARG_LIST_ACCESS_KTILE_8( (uint8_t*)list+ LIST_SIZE_KTILE(role, type, name), __VA_ARGS__ )

#define CAL_ARG_COUNT_LIST_ELEMENTS(_var, numArgs, ...) numArgs

#endif // _CAL_KArgs_
