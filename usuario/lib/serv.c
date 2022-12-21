/*
 *  usuario/lib/serv.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *
 */

/*
 *
 * Fichero que contiene las definiciones de las funciones de interfaz
 * a las llamadas al sistema. Usa la funcion de apoyo llamsis
 *
 *      SE DEBE MODIFICAR AL INCLUIR NUEVAS LLAMADAS
 *
 */

#include "llamsis.h"
#include "servicios.h"

/* Función del módulo "misc" que prepara el código de la llamada
   (en el registro 0), los parámetros (en registros 1, 2, ...), realiza la
   instruccion de llamada al sistema  y devuelve el resultado 
   (que obtiene del registro 0) */

int llamsis(int llamada, int nargs, ... /* args */);


/*
 *
 * Funciones interfaz a las llamadas al sistema
 *
 */

/*EJERCICIO 1 DORMIR*/
int crear_proceso(char *prog){
	return llamsis(CREAR_PROCESO, 1, (long)prog);
}
int terminar_proceso(){
	return llamsis(TERMINAR_PROCESO, 0);
}
int escribir(char *texto, unsigned int longi){
	return llamsis(ESCRIBIR, 2, (long)texto, (long)longi);
}
int obtener_id_pr(){																													//NUEVO
	return llamsis(ID_PROCESO,3);
}
int dormir(unsigned int segundos){
	return llamsis(DORMIR, 4, (long) segundos);
}

/*EJERCICIO 2 MUTEX*/
int	crear_mutex(char*nombre_mutex, int tipo_mutex){
	return llamsis(CREAR_MUTEX,5,(long)nombre_mutex,(long)tipo_mutex);
}
int abrir_mutex(char*nombre_mutex){
	return llamsis(ABRIR_MUTEX,6,(long)nombre_mutex);
}
int lock (unsigned int mutexid) {
	return llamsis(LOCK, 7, (long) mutexid);
}
int unlock (unsigned int mutexid) {
	return llamsis(UNLOCK,8,(long)mutexid);
}
int cerrar_mutex(unsigned int mutexid){
	return llamsis(CERRAR_MUTEX,9,(long)mutexid);
}
