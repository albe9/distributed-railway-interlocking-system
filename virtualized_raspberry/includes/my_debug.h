/*
*   Libreria di debug per i nodi virtualizzati, permette a ciascun nodo di stampare
*   una stringa su un file di testo apposito
*/
#ifndef INCLUDES_MY_DEBUG_H_
#define INCLUDES_MY_DEBUG_H_

#include "global_variables.h"

extern FILE *debug_file;

extern void startDebug();

#endif /* INCLUDES_MY_DEBUG_H_ */