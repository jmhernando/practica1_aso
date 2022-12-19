/*
 *  minikernel/kernel/include/llamsis.h
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero de cabecera que contiene el numero asociado a cada llamada
 *
 * 	SE DEBE MODIFICAR PARA INCLUIR NUEVAS LLAMADAS
 *
 */

#ifndef _LLAMSIS_H
#define _LLAMSIS_H

/* Numero de llamadas disponibles */
#define NSERVICIOS 9																			//NUEVO incrementar el 4 a 5 si se introduce nueva rutina

#define CREAR_PROCESO 0
#define TERMINAR_PROCESO 1
#define ESCRIBIR 2
#define	ID_PROCESO 3																		//NUEVO
#define DORMIR 4																			//NUEVO
#define CREAR_MUTEX 5																		//NUEVO															
#define	ABRIR_MUTEX 6																		//NUEVO
#define LOCK 7	
#define UNLOCK 8																			//NUEVO
#endif /* _LLAMSIS_H */

