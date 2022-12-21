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
	
// prueba 
//holita
*/
/*
 *
 * Fichero que contiene la funcionalidad del sistema operativo
 *
 */

#include "kernel.h"	/* Contiene defs. usadas por este modulo */
#include <string.h>//Para usar cadenas de caracteres
//Esto es un cambio
/*
 *
 * Funciones relacionadas con la tabla de procesos:
 *	iniciar_tabla_proc buscar_BCP_libre
 *
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
		//Rellenamos el vector descriptores con -1
		for(int i=0; i<NUM_MUT_PROC; i++){
			p_proc->descriptores[i] = -1;
		}
		int nivel = fijar_nivel_int(NIVEL_3);
		/* lo inserta al final de cola de listos */
		insertar_ultimo(&lista_listos, p_proc);
		fijar_nivel_int(nivel);
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

/*EMPEZAMOS EJERCICIO MUTEX*/
/*int caracter_max(char *nombre_Mut){
	
	//strlen cuenta los caracteres pasados como parametro
	int nCaracteres = strlen(nombre_Mut);
	if(nCaracteres>MAX_NOM_MUT){
		printk("Nombre Mut: %s\n",nombre_Mut);
		printk("Pero el nombre es demasiado largo.\n");
		return -1;
	}
	else{
		printk("Nombre Mut: %s\n",nombre_Mut);
		printk("El nombre tiene la longitud correcta.\n");
		return 0;
	}
}*/

int nombre_max(char *nombre) {
	printk("nombre: %s\n", nombre);
	
	int long_nom = strlen(nombre);
	printk("hola");
	if (long_nom > MAX_NOM_MUT) {
		printk("El nombre del mutex es demasiado largo\n");
		return -1;
	}
	else {
		
		return 0;
	}
}


//Método que comprueba si un descriptor está libre. Si lo está devuelve el número del descriptor libre.
//Si no lo está devuelve -1.
int descriptor_libre(){
	int aux = -1;
	int i = 0;

	while((aux == -1) && (i <= NUM_MUT_PROC)) {
		
		//Si descriptor = -1, no ha sido utilizado
		int x = p_proc_actual->descriptores[i];
		if(p_proc_actual->descriptores[i] == -1) {
			
			aux = i;
		}
		
		i++;
	}

	return aux;	
}


//Esta función sirve para que cuando se crea un mutex nuevo no pueda tener el mismo nombre que alguno ya creado.
int mismo_nombre(char *nombre_mutex){
	for(int i =0;i<num_mutex_creados;i++){
		if(array_mutex[i].abierto !=0){
			if((strcmp(nombre_mutex, array_mutex[i].nombre_mutex)) == 0) {
				printk("Este nombre de mutex ya está en uso.\n");
				return -1; 
			}
			printk("No hay ningún mutex con este nombre.\n");
		}
	}
	return 0;
}


//Encuentra un descriptor que esté libre
int asignar_descriptor_mutex(){
	int descriptor_encontrado = -1;
	int i=0;
	
	do{
		if(array_mutex[i].abierto==0){
			descriptor_encontrado = i;
		}
		i++;
	}while((descriptor_encontrado==-1)&&(i<NUM_MUT));
	
	return descriptor_encontrado;
}


int crear_mutex(char*nombre_mutex, int tipo_mutex){
	
	nombre_mutex=(char *)leer_registro(1);
	tipo_mutex=(int)leer_registro(2);
	//COMPROBAMOS PRIMERO SI PODEMOS CREAR EL MUTEX.
	
	int numMaxMutex = 0;
	
	int aux = nombre_max(nombre_mutex);
	if(aux == -1){
		printk("El nombre se excede de los parámetros establecidos.");
		return -1;
	}
	
	printk("Se crea un mutex\n");
	
	
	//Comprobaciones para saber si el mutex se puede crear con normalidad.
	
	//Nombre del mutex.
	if(num_mutex_creados>=1){
		int aux2 = mismo_nombre(nombre_mutex);
		
		if(aux2==-1)return -1;
	}
	
	//Comprobamos si hay descriptores libres.
	int descriptor_P = descriptor_libre();
	printk("descriptor: %d ", descriptor_P);
	if(descriptor_P == -1)return -1;
	printk("-------------------------------------------------------\n");
	
	if(num_mutex_creados == NUM_MUT) numMaxMutex = 1;
	
	while(numMaxMutex){
		printk("Numero máximo de mutex alcanzado. Se procederá a bloquear el proceso.\n");
		int nivel = fijar_nivel_int(NIVEL_3);
		
		/*Se bloquea igual que el ejercicio de dormir.*/
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
		insertar_ultimo(&lista_mutex_bloqueados, p_proc_actual);
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
		if(num_mutex_creados != NUM_MUT) numMaxMutex = 0;
	}
	
	//PODEMOS CREAR EL MUTEX.
	num_mutex_creados++;
	printk("-------------------------------------------------------\n");
	//Podemos crear ya nuestro mutex, pero antes debemos de asignarle un descriptor libre. Lo guardamos en una variable.
	int descriptorM = asignar_descriptor_mutex();
	//Y lo asignamos al mutex que lo vaya a usar.
	p_proc_actual->descriptores[descriptor_P] = descriptorM;
	printk("Asignados descriptor del proceso y del mutex: Descriptor proceso: %d. Descriptor mutex: %d",descriptor_P,descriptorM);
	
	//ACTUALIZAMOS VARIABLES DEL PROCESO (ESTRUCTURA BCP)
	
	//Solo necesitamos actualizar el número de descriptores que hay en nuestro proceso.
	//La variable num_descriptores es nueva, se declara en KERNEL.H
	p_proc_actual ->num_descriptores++;
	
	//ACTUALIZAMOS VARIABLES DEL MUTEX (ESTRUCTURA MUTEX)
	
	//Le asignamos el nombre que pasamos por parámetro en la función crear mutex. (esta funcion)
	array_mutex[descriptorM].nombre_mutex = nombre_mutex;	
	//Le asignamos el tipo que también pasamos por parámetro.
	array_mutex[descriptorM].tipo_mutex = tipo_mutex;
	//Le asignamos el valor de 1 para saber que se está usando.
	array_mutex[descriptorM].abierto =1;
	printk("Un nuevo mutex se ha creado. Número de mutex actualmente en funcionamiento: %d\n",num_mutex_creados);
	return descriptor_P;
	
}

//Compara el nombre del mutex con uno que ya esté declarado en el array de mutex y devuelve su descriptor

int descriptor_mutex(char*nombre_mutex){
	int nuevo_descriptor_mutex;
	for(int i = 0; i<num_mutex_creados;i++){
		if(strcmp(array_mutex[i].nombre_mutex,nombre_mutex)==0){
			nuevo_descriptor_mutex = i;
		}
	}
	return nuevo_descriptor_mutex;
}
int abrir_mutex(char*nombre_mutex){
	nombre_mutex=(char *)leer_registro(1);
	
	int aux = nombre_max(nombre_mutex);
	printk("hola");
	if(aux == -1){
		printk("El nombre se excede de los parámetros establecidos.");
		return -1;
	}
	
	
	
	
	
	//Comprobaciones para saber si el mutex se puede crear con normalidad.
	
	//Nombre del mutex.
	if(num_mutex_creados>=1){
		int aux2 = mismo_nombre(nombre_mutex);
		if(aux2==-1)return -1;
	}
	
	//Comprobamos si hay descriptores libres.
	int descriptor_P = descriptor_libre();
	if(descriptor_P == -1)return -1;
	
	int nuevo_descriptor_mutex;
	//Hemos creado una función nueva para esta instrucción. Está mas arriba, descriptor_mutex()
	nuevo_descriptor_mutex = descriptor_mutex(nombre_mutex);
	int descriptorM = nuevo_descriptor_mutex;
	printk("El mutex se abre correctamente\n");
	
	p_proc_actual->descriptores[descriptor_P] = descriptorM;
	printk("Asignados descriptor del proceso y del mutex: Descriptor proceso: %d. Descriptor mutex: %d",descriptor_P,descriptorM);
	
	//Solo necesitamos actualizar el número de descriptores que hay en nuestro proceso.
	p_proc_actual->num_descriptores++;
	//Le asignamos el valor de 1 para saber que se está usando.
	array_mutex[descriptorM].abierto ++;
	return descriptor_P;
	
}

int lock(unsigned int mutexid){
	int descriptor_proc = (unsigned int)leer_registro(1);
	char aux = 'S';
	int aux2;
	
	mutexid = p_proc_actual -> descriptores[descriptor_proc];
	
	if(mutexid == -1){
		printk("Error, debe de crearse el mutex antes de bloquearlo");
		return -1;
	}
	
	printf("Id del mutex: %d.\n Descriptor del proceso: %d\n",mutexid,descriptor_proc);
	if((array_mutex[mutexid].tipo_mutex) == RECURSIVO){
		aux2 = 0;
	}
	else{
		aux2 = 1;
	}
	
	while(aux=='S'){
		if(array_mutex[mutexid].bloquear>0){
			switch(aux2){
					//Case 0 para cuando es recursivo
				case 0: 
					printk("Caso en el que SI es recursivo.");
					//Cuando el proceso actual bloquea el mutex, añadimos un bloqueo mas al array de mutex.
					
					if(array_mutex[mutexid].proceso_bloqueante==p_proc_actual ->id){
						printk("Se bloquea el mutex al ser un proceso recursivo.");
						//Como el proceso actual es el que bloquea el mutex, se bloquea sin necesidad de bloquear también el proceso.					
						array_mutex[mutexid].bloquear++;
						//Cuando aux toma el valor de N significa que no vamos a volver a iterar en el bucle while.
						aux = 'N';
					}
					
					//Si es otro proceso el que intenta bloquear el mutex...
					else{
						int nivel = fijar_nivel_int(NIVEL_3);
						printk("Id del proceso: %d. Se va a bloquear\n", p_proc_actual->id);

						/*Pone al proceso actual en estado bloqueado.*/
						p_proc_actual->estado=BLOQUEADO;
						//Puntero que apunta al BCP, se llamará proceso bloqueado y con valor el proceso actual.
						BCP* p_proc_bloqueado = p_proc_actual;


						//ELIMINAR PROCESO DE LA LISTA DE LISTOS E INTRODUCIRLO EN LA LISTA DE BLOQUEADOS.

						/*Al estar bloqueado ya no está listo, por lo tanto se le expulsa de dicha lista, lo guarda en la de bloqueados.*/
						/*El BCP dispone del puntero siguiente, que permite que se crean listas, en nuestro caso solo tenemos la lista de listos
						que agrupa los procesos listos para ejecutar. (El que se ejecuta actualmente también está en esta lista.)*/
						eliminar_primero(&lista_listos);
						/*Se pone en la lista de dormidos el proceso actual que se ha pasado a estado bloqueado.*/
						insertar_ultimo(&(array_mutex[mutexid].lista_esperando_bloqueo), p_proc_actual);
						//Tanto eliminar_primero e insertar_ultimo son funciones de esta misma clase.


						//COGER UN NUEVO PROCESO Y HACER CAMBIO DE CONTEXTO.

						/*Función ya perteneciente a esta clase. Mientras que haya un proceso listo, lo devuelve. (Siempre devuelve el primero de la lista)*/
						p_proc_actual= planificador();
						/*Al hacer un cambio de proceso en ejecución se debe hacer un cambio de contexto:*/
						printk("Se hace un cambio de contexto del proceso: %d al proceso: %d \n", p_proc_bloqueado->id,p_proc_actual->id);
						/*cambio_contexto(contexto_t*contexto_a_salvar,contexto_t*contexto_a_restaurar)
							esta función consiste en copiar el estado actual de los registros del procesador en el primer parámetro.
							El siguiente parámetro hace la función inversa.

						*/
						cambio_contexto(&(p_proc_bloqueado->contexto_regs),&(p_proc_actual->contexto_regs));
						/*Volver al nivel de interrupción anterior.*/
						fijar_nivel_int(nivel);
					}
					break;
					//Caso 1 para cuando no es recursivo, significa que no se puede volver a bloquear el mutex.
				case 1: 
					printk("Caso en el que NO es recursivo.");
					if(array_mutex[mutexid].proceso_bloqueante==p_proc_actual ->id){
						//Como no es recursivo, si ya ha sido bloqueado no puede volver a serlo.
						//Por lo tanto mensaje de error y devuelve -1;
						printk("Error al intentar bloquear un mutex ya bloqueado (Y NO RECURSIVO)");
						return -1;
					}
					else{
						
					//Como la función de este else es la misma que el anterior, el código es el mismo.
						int nivel = fijar_nivel_int(NIVEL_3);
						printk("Id del proceso: %d. Se va a bloquear\n", p_proc_actual->id);

						/*Pone al proceso actual en estado bloqueado.*/
						p_proc_actual->estado=BLOQUEADO;
						//Puntero que apunta al BCP, se llamará proceso bloqueado y con valor el proceso actual.
						BCP* p_proc_bloqueado = p_proc_actual;


						//ELIMINAR PROCESO DE LA LISTA DE LISTOS E INTRODUCIRLO EN LA LISTA DE BLOQUEADOS.

						/*Al estar bloqueado ya no está listo, por lo tanto se le expulsa de dicha lista, lo guarda en la de bloqueados.*/
						/*El BCP dispone del puntero siguiente, que permite que se crean listas, en nuestro caso solo tenemos la lista de listos
						que agrupa los procesos listos para ejecutar. (El que se ejecuta actualmente también está en esta lista.)*/
						eliminar_primero(&lista_listos);
						/*Se pone en la lista de dormidos el proceso actual que se ha pasado a estado bloqueado.*/
						insertar_ultimo(&(array_mutex[mutexid].lista_esperando_bloqueo), p_proc_actual);
						//Tanto eliminar_primero e insertar_ultimo son funciones de esta misma clase.


						//COGER UN NUEVO PROCESO Y HACER CAMBIO DE CONTEXTO.

						/*Función ya perteneciente a esta clase. Mientras que haya un proceso listo, lo devuelve. (Siempre devuelve el primero de la lista)*/
						p_proc_actual= planificador();
						/*Al hacer un cambio de proceso en ejecución se debe hacer un cambio de contexto:*/
						printk("Se hace un cambio de contexto del proceso: %d al proceso: %d \n", p_proc_bloqueado->id,p_proc_actual->id);
						/*cambio_contexto(contexto_t*contexto_a_salvar,contexto_t*contexto_a_restaurar)
							esta función consiste en copiar el estado actual de los registros del procesador en el primer parámetro.
							El siguiente parámetro hace la función inversa.

						*/
						cambio_contexto(&(p_proc_bloqueado->contexto_regs),&(p_proc_actual->contexto_regs));
						/*Volver al nivel de interrupción anterior.*/
						fijar_nivel_int(nivel);
					}
					break;
					
			}
		}
		else{
			aux ='N';
			array_mutex[mutexid].bloquear++;
		}
	}
	array_mutex[mutexid].proceso_bloqueante = p_proc_actual->id;
	printk("El bloqueo se ha resuelto exitosamente.\n");
	//El return 0 es cuando una función funciona sin errores.
	return 0;
}
int unlock (unsigned int mutexid) {
	
	int aux2;
	int descriptor_proc = (unsigned int)leer_registro(1);
	mutexid = p_proc_actual -> descriptores[descriptor_proc];
	
	if(mutexid == -1){
		printk("Error, debe de crearse el mutex antes de bloquearlo\n");
		return -1;
	}
	if((array_mutex[mutexid].tipo_mutex) == RECURSIVO){
		aux2 = 0;
	}
	else{
		aux2 = 1;
	}
	if(array_mutex[mutexid].bloquear>0){
		switch(aux2){
					//Case 0 para cuando es recursivo
				case 0: 
					if(array_mutex[mutexid].proceso_bloqueante==p_proc_actual ->id){
						printk("Es el proceso actual quien libera el mutex.\n");
						//Como el proceso actual es el que bloquea el mutex, se bloquea sin necesidad de bloquear también el proceso.					
						array_mutex[mutexid].bloquear--;
						if(array_mutex[mutexid].bloquear == 0){
							//Si hay mas procesos que esperan bloqueo en la lista, también se deben desbloquear.
							if(((array_mutex[mutexid].lista_esperando_bloqueo).primero) != NULL){
								int nivel = fijar_nivel_int(NIVEL_3);
								BCP* proceso_esperando_mutex = (array_mutex[mutexid].lista_esperando_bloqueo).primero;
								proceso_esperando_mutex->estado = LISTO;
								eliminar_primero(&(array_mutex[mutexid].lista_esperando_bloqueo));
								insertar_ultimo(&lista_listos,proceso_esperando_mutex);
								fijar_nivel_int(nivel);
								printk("Hemos desbloqueado el proceso con id: %d\n", proceso_esperando_mutex->id);
							}
							array_mutex[mutexid].proceso_bloqueante = -1;
						}
					}
					else{
						printk("Este proceso no tiene bloqueado ningún mutex.\n");
						return -1;
					}
				break;
				
			case 1:
				if(array_mutex[mutexid].proceso_bloqueante==p_proc_actual ->id){
						printk("Es el proceso actual quien libera el mutex.\n");
						//Como el proceso actual es el que bloquea el mutex, se bloquea sin necesidad de bloquear también el proceso.					
						array_mutex[mutexid].bloquear--;
						array_mutex[mutexid].proceso_bloqueante = -1;
							//Si hay mas procesos que esperan bloqueo en la lista, también se deben desbloquear.
						if(((array_mutex[mutexid].lista_esperando_bloqueo).primero) != NULL){
							int nivel = fijar_nivel_int(NIVEL_3);
							BCP* proceso_esperando_mutex = (array_mutex[mutexid].lista_esperando_bloqueo).primero;
							proceso_esperando_mutex->estado = LISTO;
							eliminar_primero(&(array_mutex[mutexid].lista_esperando_bloqueo));
							insertar_ultimo(&lista_listos,proceso_esperando_mutex);
							fijar_nivel_int(nivel);
							printk("Hemos desbloqueado el proceso con id: %d\n", proceso_esperando_mutex->id);
						}
					}
				else{
					printk("Este proceso no tiene bloqueado ningún mutex.\n");
					return -1;
				}
				break;
		}
	}
	else{
		printk("Este proceso no tiene bloqueado ningún mutex.\n");
		return -1;
	}
	printk("Se ha realizado el desbloqueo.\n");
	return 0;
	
}
int cerrar_mutex(unsigned int mutexid){
	printk("Se va a cerrar el mutex\n");
	unsigned int aux = (unsigned int)leer_registro(1);
	if(aux<16){
		mutexid = aux;
	}
	int descriptor_mutex = p_proc_actual -> descriptores[mutexid];
	printk("Descriptor del mutex: %d\n", descriptor_mutex);
	
	//Comprobamos si existe un descriptor del mutex. Si no lo hay significa que o no se creo o ya está cerrado.
	if(array_mutex[descriptor_mutex].nombre_mutex == NULL){
		printk("El nombre del descriptor no existe.\n");
		return -1;
	}
	//Como no se ha cerrado todavía ponemos entonces el valor de -1 para que se cierre.
	p_proc_actual->descriptores[mutexid] = -1;
	p_proc_actual->num_descriptores--;
	//printk("Después de la operación de cerrar descriptores quedan: %s descriptores \n", num_descriptores);
	
	if(array_mutex[descriptor_mutex].proceso_bloqueante == p_proc_actual->id) {
		printk("Llamamos a la función unlock, puesto que existe un el mutex está bloqueado por el proceso que quiere terminarlo\n");
		unlock(mutexid);
	}
	
	array_mutex[descriptor_mutex].abierto--;
	
	//el mutex se ha cerrado ya, ahora hay que eliminarlo
	printk("Mutex cerrado, se procede a su eliminación \n");
	
	//Como varios procesos pueden abrir un mutex se comprueba que esto no haya sucedido.
	if(array_mutex[descriptor_mutex].abierto <= 0) {
		
		num_mutex_creados--;
		int aux2 = 1;
		while(lista_mutex_bloqueados.primero != NULL){
			
			int nivel = fijar_nivel_int(NIVEL_3);
			BCP* proceso_esperando_mutex = lista_mutex_bloqueados.primero;
			proceso_esperando_mutex->estado = LISTO;
			eliminar_primero(&lista_mutex_bloqueados);
			insertar_ultimo(&lista_listos,proceso_esperando_mutex);
			fijar_nivel_int(nivel);
			printk("Se ha desbloqueado: %d proceso \n", aux2);
			aux2++;
		}
		
		
	}
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

	
	
	if (p_proc_actual->num_descriptores > 0) {
		printk("Cerrar mutex que ha abierto el proceso\n");
		for(int i = 0; i < NUM_MUT_PROC; i++) {
			printk("%d\n", i);
			cerrar_mutex(i);			
		}
		printk("se ha terminado de cerrar los mutex\n");
	}
	
	
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
