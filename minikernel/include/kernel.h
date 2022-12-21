/*
 *  minikernel/include/kernel.h
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene definiciones usadas por kernel.c
 *
 *      SE DEBE MODIFICAR PARA INCLUIR NUEVA FUNCIONALIDAD
 *
 */
/*kernel.h contiene definiciones usadas por jernel.c como la del bcp. Este archivo DEBE ser modificado para añadir nuevos campos al bcp*/
#ifndef _KERNEL_H
#define _KERNEL_H

#include "const.h"
#include "HAL.h"
#include "llamsis.h"
#include <string.h>

/*
 *
 * Definicion del tipo que corresponde con el BCP.
 * Se va a modificar al incluir la funcionalidad pedida.
 *
 */
typedef struct BCP_t *BCPptr;

typedef struct BCP_t {
        int id;				/* ident. del proceso */
        int estado;			/* TERMINADO|LISTO|EJECUCION|BLOQUEADO*/
        contexto_t contexto_regs;	/* copia de regs. de UCP */
        void * pila;			/* dir. inicial de la pila */
		int tiempo_dormido; //Tiempo que debe permanecer dormido el proceso del ejercicio 1.   							VARIABLE NUEVA.
		int num_descriptores;	//Numero de descriptores del proceso.
		int descriptores[NUM_MUT_PROC];	//Array de descriptores
															
		BCPptr siguiente;		/* puntero a otro BCP */
		void *info_mem;			/* descriptor del mapa de memoria */
} BCP;

/*
 *
 * Definicion del tipo que corresponde con la cabecera de una lista
 * de BCPs. Este tipo se puede usar para diversas listas (procesos listos,
 * procesos bloqueados en semáforo, etc.).
 *
 */

typedef struct{
	BCP *primero;
	BCP *ultimo;
} lista_BCPs;

#define NO_RECURSIVO 0
#define RECURSIVO 1

typedef struct{
	//Las necesitamos para crear mutex.Las necesitamos para cerrar.
	char* nombre_mutex;
	int tipo_mutex;
	int proceso_bloqueante;
	int abierto;
	int bloquear;					

	lista_BCPs lista_esperando_bloqueo;
	
}mutex;


																	//Nuevo array de mutex
mutex array_mutex[NUM_MUT];
int num_mutex_creados;												//NUEVA VARIABLE usada en crear_mutex
/*
 * Variable global que identifica el proceso actual
 */

BCP* p_proc_actual=NULL;

/*
 * Variable global que representa la tabla de procesos
 */

BCP tabla_procs[MAX_PROC];

/*
 * Variable global que representa la cola de procesos listos
 */
lista_BCPs lista_listos= {NULL, NULL};

lista_BCPs lista_dormir ={NULL,NULL};		

lista_BCPs lista_mutex_bloqueados ={NULL,NULL};	
//VARIABLE NUEVA
/*
 *
 * Definición del tipo que corresponde con una entrada en la tabla de
 * llamadas al sistema.
 *
 */
typedef struct{
	int (*fservicio)();
} servicio;


/*
 * Prototipos de las rutinas que realizan cada llamada al sistema
 */
int sis_crear_proceso();
int sis_terminar_proceso();
int sis_escribir();	
	
/*Al crear en kernel.c una nueva rutina, esta se debe declarar en este archivo.*/
int obtener_id_pr();																				//NUEVO
int dormir(unsigned int segundos);																	//NUEVO


int	crear_mutex(char*nombre_mutex, int tipo_mutex);													//NUEVO
int abrir_mutex(char*nombre_mutex);																	//NUEVO
int lock(unsigned int mutexid);	
int unlock (unsigned int mutexid);																//NUEVO
int cerrar_mutex (unsigned int mutexid);																//NUEVO
/*
 * Variable global que contiene las rutinas que realizan cada llamada
 */
	
servicio tabla_servicios[NSERVICIOS]={	{sis_crear_proceso},
					{sis_terminar_proceso},
					{sis_escribir},
					{obtener_id_pr}, 	//rutina que ofrece el id.									//NUEVO EJERCICIO 1
					{dormir},															  			//NUEVO EJERCICIO 1
					{crear_mutex},																	//NUEVO EJERCICIO 2
					{abrir_mutex},																	//NUEVO EJERCICIO 2
					{lock},
					{unlock},																		//NUEVO EJERCICIO 2
					{cerrar_mutex}
					 };

#endif /* _KERNEL_H */

