#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include "timer.h"
#include "CAL.h"


//################################################# (Desde 20-01-2018) SuperComputing

//***************** Black Schole (embarrasing)

//Aplicacion de economía, donde se repite muchas veces un mismo kernel sobre conjunto de datos diferentes.
BlackCtrlBase: Es el base, el de Ana, con controladores y una única GPU. Nada de comunicaciones con Hitmap

BlackSupCtrl:  Desarrollado por mí para soportar Multi GPU con MPI. Un controlador por GPU. Es embarrasing parallel. 
Se divide el número de kernels entre el nº de GPUs y así, calculamos el nº de kernels por GPU.  
No hay comunicaciones Hitmap ya que es embarrasing.

//***************** Recurrence Equation (embarrasing)

recEqCont: Recurrence Equation base, para el europar, una GPU un MMPI, un solo kernel. Versión sincrona y asincrona.

recEqSupCtrl: Suma de matrices con iteraciones internas y una multiplicacion de un float. Es embarrasin ya que cada elemento se 
hace por separado. El espacio se divide por el nº de GPUs. No hay comunicaciones pero se particiona el espacio de cómputo.

//***************** Cannon MM (Comunicadores)

cannonSupCtrl: Multipicación de matrices en cannon con comunicares de hitmap. Versión síncrona. Un controlador 
por GPU etc.

//***************** jacobi2D más conocido como stencil (Comunicadores)

jacobi2DSupCtrl: Stencil con comunicadores, un controller por cada GPU
