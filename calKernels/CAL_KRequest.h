/*
 * CAL_KRequest: Macros to generate the code, from the kernel dependecies to other kernels launches.
 *
 * @version 1.2
 * @author Ismael Taboada Rodero
 * @date Jan 2018
 * 
 */
#ifndef _CAL_KRequest_
#define _CAL_KRequest_

typedef struct{
    void* stream;
    void* event;
} CAL_Request;

#define CAL_REQUEST_NULL NULL

#endif // _CAL_KRequest_