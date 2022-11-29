/*
 *  kernel/kernel.c
 *
 *  Minikernel. Versión 1.0
 *
 *  Fernando Pérez Costoya
 *	
 */
/*Trabajo realizado por:
	Iván Maqueda Guede
	Laura Sánchez Muñoz 
	Jose Manuel hernando Sánchez
*/
/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */
//Esto es un cambio
/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
 */

/*
 * Función que inicia la tabla de procesos
 */
static void iniciar_tabla_proc(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		tabla_procs[i].estado=NO_USADA;
}

/*
 * Función que busca una entrada libre en la tabla de procesos
 */
static int buscar_BCP_libre(){
	int i;

	for (i=0; i<MAX_PROC; i++)
		if (tabla_procs[i].estado==NO_USADA)
			return i;
	return -1;
}

/*
 *
 * Funciones que facilitan el manejo de las listas de BCPs
 *	insertar_ultimo eliminar_primero eliminar_elem
 *
 * NOTA: PRIMERO SE DEBE LLAMAR A eliminar Y LUEGO A insertar
 */

/*
 * Inserta un BCP al final de la lista.
 */
static void insertar_ultimo(lista_BCPs *lista, BCP * proc){
	if (lista->primero==NULL)
		lista->primero= proc;
	else
		lista->ultimo->siguiente=proc;
	lista->ultimo= proc;
	proc->siguiente=NULL;
}

/*
 * Elimina el primer BCP de la lista.
 */
static void eliminar_primero(lista_BCPs *lista){

	if (lista->ultimo==lista->primero)
		lista->ultimo=NULL;
	lista->primero=lista->primero->siguiente;
}

/*
 * Elimina un determinado BCP de la lista.
 */
static void eliminar_elem(lista_BCPs *lista, BCP * proc){
	BCP *paux=lista->primero;

	if (paux==proc)
		eliminar_primero(lista);
	else {
		for ( ; ((paux) && (paux->siguiente!=proc));
			paux=paux->siguiente);
		if (paux) {
			if (lista->ultimo==paux->siguiente)
				lista->ultimo=paux;
			paux->siguiente=paux->siguiente->siguiente;
		}
	}
}

/*
 *
 * Funciones relacionadas con la planificacion
 *	espera_int planificador
 */

/*
 * Espera a que se produzca una interrupcion
 */
static void espera_int(){
	int nivel;

	printk("-> NO HAY LISTOS. ESPERA INT\n");

	/* Baja al mínimo el nivel de interrupción mientras espera */
	nivel=fijar_nivel_int(NIVEL_1);
	halt();											//Para que se usa halt
	fijar_nivel_int(nivel);
}

/*
 * Función de planificacion que implementa un algoritmo FIFO.
 */
static BCP * planificador(){
	while (lista_listos.primero==NULL)
		espera_int();		/* No hay nada que hacer */
	return lista_listos.primero;
}

/*
 *
 * Funcion auxiliar que termina proceso actual liberando sus recursos.
 * Usada por llamada terminar_proceso y por rutinas que tratan excepciones
 *
 */
static void liberar_proceso(){
	BCP * p_proc_anterior;

	liberar_imagen(p_proc_actual->info_mem); /* liberar mapa */

	p_proc_actual->estado=TERMINADO;
	eliminar_primero(&lista_listos); /* proc. fuera de listos */

	/* Realizar cambio de contexto */
	p_proc_anterior=p_proc_actual;
	p_proc_actual=planificador();

	printk("-> C.CONTEXTO POR FIN: de %d a %d\n",
			p_proc_anterior->id, p_proc_actual->id);

	liberar_pila(p_proc_anterior->pila);
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
        return; /* no debería llegar aqui */
}

/*
 *
 * Funciones relacionadas con el tratamiento de interrupciones
 *	excepciones: exc_arit exc_mem
 *	interrupciones de reloj: int_reloj
 *	interrupciones del terminal: int_terminal
 *	llamadas al sistemas: llam_sis
 *	interrupciones SW: int_sw
 *
 */

/*
 * Tratamiento de excepciones aritmeticas
 */
static void exc_arit(){

	if (!viene_de_modo_usuario())
		panico("excepcion aritmetica cuando estaba dentro del kernel");


	printk("-> EXCEPCION ARITMETICA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de excepciones en el acceso a memoria
 */
static void exc_mem(){

	if (!viene_de_modo_usuario())
		panico("excepcion de memoria cuando estaba dentro del kernel");


	printk("-> EXCEPCION DE MEMORIA EN PROC %d\n", p_proc_actual->id);
	liberar_proceso();

        return; /* no debería llegar aqui */
}

/*
 * Tratamiento de interrupciones de terminal
 */
 
 
 
 
 
 
 
 /*Hay que modificar esta función para que lea caracteres del teclado y guardarlos en el buffer Ejercicio 5*/					//AÑADIR
 
static void int_terminal(){
	char car;

	car = leer_puerto(DIR_TERMINAL);
	printk("-> TRATANDO INT. DE TERMINAL %c\n", car);

        return;
}

/*
 * Tratamiento de interrupciones de reloj
 */
static void int_reloj(){

	printk("-> TRATANDO INT. DE RELOJ\n");
																												//NUEVO
		BCP* proceso = lista_dormir.primero;
		
	while (proceso != NULL) {
		//Disminuir su tiempo
		proceso->tiempo_dormido--;
		printk("Al proceso con id = %d le queda %d\n", proceso-> id, proceso->tiempo_dormido);
		//Si el plazo ha acabado, desbloquear
		//Antes de borrarlo hay que guardar el siguiente proceso al que apunta en la cola dormidos
		//Si se borra despues, apunta a otro que no esta dormido!!
		BCP* proceso_siguiente = proceso->siguiente;
		if(proceso->tiempo_dormido <= 0) {
			//Elevar nivel interrupcion y guardar actual (desconocido)
			int nivel = fijar_nivel_int(NIVEL_3); //Inhibir int reloj mientras se manejan listas
			
			printk("El proceso con id = %d despierta\n", proceso->id);
			//Cambiar el estado
			proceso->estado = LISTO;
			//Quitarlo de la lista dormidos
			eliminar_elem(&lista_dormir, proceso);
			//Ponerlo el ultimo en la cola de listos
			insertar_ultimo(&lista_listos, proceso);

			//Volver al nivel de interrupcion anterior
			fijar_nivel_int(nivel);		
		}
		//Pasar al siguiente elemento en la cola de dormidos
		proceso = proceso_siguiente;
	}
        return;
}

/*
 * Tratamiento de llamadas al sistema
 */
static void tratar_llamsis(){
	int nserv, res;

	nserv=leer_registro(0);
	if (nserv<NSERVICIOS)
		res=(tabla_servicios[nserv].fservicio)();
	else
		res=-1;		/* servicio no existente */
	escribir_registro(0,res);
	return;
}

/*
 * Tratamiento de interrupciuones software
 */
static void int_sw(){

	printk("-> TRATANDO INT. SW\n");

	return;
}

/*
 *
 * Funcion auxiliar que crea un proceso reservando sus recursos.
 * Usada por llamada crear_proceso.
 *
 */
static int crear_tarea(char *prog){
	void * imagen, *pc_inicial;
	int error=0;
	int proc;
	BCP *p_proc;

	proc=buscar_BCP_libre();
	if (proc==-1)
		return -1;	/* no hay entrada libre */

	/* A rellenar el BCP ... */
	p_proc=&(tabla_procs[proc]);

	/* crea la imagen de memoria leyendo ejecutable */
	imagen=crear_imagen(prog, &pc_inicial);
	if (imagen)
	{
		p_proc->info_mem=imagen;
		p_proc->pila=crear_pila(TAM_PILA);
		fijar_contexto_ini(p_proc->info_mem, p_proc->pila, TAM_PILA,
			pc_inicial,
			&(p_proc->contexto_regs));
		p_proc->id=proc;
		p_proc->estado=LISTO;

		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		error= 0;
	}
	else
		error= -1; /* fallo al crear imagen */

	return error;
}

/*
 *
 * Rutinas que llevan a cabo las llamadas al sistema
 *	sis_crear_proceso sis_escribir
 *
 */




/*Llamada para obtener el id del proceso que la invoca*/															//AÑADIDA



//Este método debe llamarse así porque en la clase de prueba yosoy.c llama a esta función.
int obtener_id_pr(){
	int id = p_proc_actual->id;
	printk("Id del proceso actual\n", id);
	return id;
}

//Archivos a cambiar para una llamada a sistema: KERNEL.C, KERNEL.H, LLAMSIS.H. SERV.C, SERVICIOS.H
//Para hacer la prueba de este método hay que descomentar en init.c

//Llamada que bloquea un proceso un plazo de tiempo, se pasa por parámetro el tiempo en milisegundos.				//NUEVO

int dormir(unsigned int segundos){
	//leer_registro es una rutina que permite leer y escribir en lso registros de propósito general del procesador
	segundos = (unsigned int)leer_registro(1);		
	/*Esta función fija el nivel de interrupcion del procesador devolviendo el rpevio. 
	Entonces permite que haya interrupciones de niveles superiores y no de inferiores. Se puede volver al estado
	original usando esta función*/
	int nivel = fijar_nivel_int(NIVEL_3);
	printk("Id del proceso: %d. Se va a bloquear: %d segundos\n", p_proc_actual->id, segundos);
	
	//PONEMOS EL PROCESO EN ESTADO BLOQUEADO DURANTE X SEGUNDOS.
	
	
	/*Pone a dormir el proceso actual tantos segundos.*/
	p_proc_actual->tiempo_dormido=segundos*TICK;
	/*Pone al proceso actual en estado bloqueado.*/
	p_proc_actual->estado=BLOQUEADO;
	//Puntero que apunta al BCP, se llamará proceso dormido y con valor el proceso actual.
	BCP* p_proc_dormido = p_proc_actual;
	
	
	//ELIMINAR PROCESO DE LA LISTA DE LISTOS E INTRODUCIRLO EN LA LISTA DE BLOQUEADOS.
	
	/*Al estar bloqueado ya no está listo, por lo tanto se le expulsa de dicha lista, lo guarda en la de bloqueados.*/
	/*El BCP dispone del puntero siguiente, que permite que se crean listas, en nuestro caso solo tenemos la lista de listos
	que agrupa los procesos listos para ejecutar. (El que se ejecuta actualmente también está en esta lista.)*/
	eliminar_primero(&lista_listos);
	/*Se pone en la lista de dormidos el proceso actual que se ha pasado a estado bloqueado.*/
	insertar_ultimo(&lista_dormir, p_proc_dormido);
	//Tanto eliminar_primero e insertar_ultimo son funciones de esta misma clase.
	
	
	//COGER UN NUEVO PROCESO Y HACER CAMBIO DE CONTEXTO.
	
	/*Función ya perteneciente a esta clase. Mientras que haya un proceso listo, lo devuelve. (Siempre devuelve el primero de la lista)*/
	p_proc_actual= planificador();
	/*Al hacer un cambio de proceso en ejecución se debe hacer un cambio de contexto:*/
	printk("Se hace un cambio de contexto del proceso: %d al proceso: %d \n", p_proc_dormido->id,p_proc_actual->id);
	/*cambio_contexto(contexto_t*contexto_a_salvar,contexto_t*contexto_a_restaurar)
		esta función consiste en copiar el estado actual de los registros del procesador en el primer parámetro.
		El siguiente parámetro hace la función inversa.
		
	*/
	cambio_contexto(&(p_proc_dormido->contexto_regs),&(p_proc_actual->contexto_regs));
	/*Volver al nivel de interrupción anterior.*/
	fijar_nivel_int(nivel);
	return 0;
}



/*
 * Tratamiento de llamada al sistema crear_proceso. Llama a la
 * funcion auxiliar crear_tarea sis_terminar_proceso
 */
 
int sis_crear_proceso(){
	char *prog;
	int res;

	printk("-> PROC %d: CREAR PROCESO\n", p_proc_actual->id);
	prog=(char *)leer_registro(1);
	res=crear_tarea(prog);
	return res;
}

/*
 * Tratamiento de llamada al sistema escribir. Llama simplemente a la
 * funcion de apoyo escribir_ker
 */
int sis_escribir()
{
	char *texto;
	unsigned int longi;

	texto=(char *)leer_registro(1);
	longi=(unsigned int)leer_registro(2);

	escribir_ker(texto, longi);
	return 0;
}

/*
 * Tratamiento de llamada al sistema terminar_proceso. Llama a la
 * funcion auxiliar liberar_proceso
 */
int sis_terminar_proceso(){

	printk("-> FIN PROCESO %d\n", p_proc_actual->id);

	liberar_proceso();

        return 0; /* no debería llegar aqui */
}

/*
 *
 * Rutina de inicialización invocada en arranque
 *
 */
int main(){
	/* se llega con las interrupciones prohibidas */

	instal_man_int(EXC_ARITM, exc_arit); 
	instal_man_int(EXC_MEM, exc_mem); 
	instal_man_int(INT_RELOJ, int_reloj); 
	instal_man_int(INT_TERMINAL, int_terminal); 
	instal_man_int(LLAM_SIS, tratar_llamsis); 
	instal_man_int(INT_SW, int_sw); 

	iniciar_cont_int();		/* inicia cont. interr. */
	iniciar_cont_reloj(TICK);	/* fija frecuencia del reloj */
	iniciar_cont_teclado();		/* inici cont. teclado */

	iniciar_tabla_proc();		/* inicia BCPs de tabla de procesos */

	/* crea proceso inicial */
	if (crear_tarea((void *)"init")<0)
		panico("no encontrado el proceso inicial");
	
	/* activa proceso inicial */
	p_proc_actual=planificador();
	cambio_contexto(NULL, &(p_proc_actual->contexto_regs));
	panico("S.O. reactivado inesperadamente");
	return 0;
}
