#include "udp.h"

/*Valor para que sea trafico de tipo UDP*/
#define UDP_IPv4_TYPE 17
/*Valor maximo del puerto random de conexion que calculamos*/
#define MAX_RAND_PORT 9000

/*Estructura basica de un header datagrama UDP*/
typedef struct udp_datagram {
	uint16_t port_src; 		//valor a 0 por default, salvo que calculemos uno
	uint16_t port_dst;
	uint16_t length; 		// longitud en octetos de este datagrama incluyendo este header y los datos (minimum value=8)
	uint16_t checksum; 		//¿tenemos funcion para esto?
	unsigned char udp_payload[UDP_MAX_LENGTH];
} udp_dtg_t;

/*Estructura de un pseudoheader datagrama UDP*/
typedef struct udp_pseudoheader{
	ipv4_addr_t source_ip;
	ipv4_addr_t destination_ip;
	uint8_t zeros;
	uint8_t proto;
	uint8_t udp_length;
	udp_dtg_t udp_header;
} udp_pseudoh_t;

/*variables globales*/
uint16_t my_port;


/*
 * int udp_open(char *config, char *rtable,uint16_t port);
 *
 * DESCRIPCIÓN:
 *   Esta función abre una conexion UDP para enviar paquetes.
 *
 * PARÁMETROS:
 *   'config': Puntero al file donde esta guardada la configuracion
 *   'rtable': Puntero al file donde esta guardada la routing table
 *	 'port': Puerto que queda a la escucha. Si el argumento es 0 se genera uno aleatorio.
 * VALOR DEVUELTO:
 *   El valor es '0' si la conexion udp ha sido abierta correctamente.
 *
 * ERRORES:
 *   La función devuelve err_code que sera !=0 si algo no ha ocurrido como lo esperado
 */

int udp_open(char *config, char *rtable,uint16_t port){

	unsigned int seed = time(NULL);
	srand(seed);

	int err_code = ipv4_open(config, rtable);

	if(port == 0){
		my_port = get_rnd_port();
	}
	else{
		my_port = port;
	}
	printf("Abierta interfaz UDP.\n");
	return err_code;
}


/*
 * int udp_close();
 *
 * DESCRIPCIÓN:
 *   Esta función cierra una conexion UDP.
 *
 * VALOR DEVUELTO:
 *   El valor es '0' si la conexion udp ha sido cerrada correctamente.
 *
 * ERRORES:
 *   La función devuelve -1 si no ha podido cerrar la interfaz eth o ipv4.
 */
int udp_close(){
	return ipv4_close();
}


/*
 * int udp_send(ipv4_addr_t dst_addr,uint16_t port, unsigned char * payload, int payload_len);
 *
 * DESCRIPCIÓN:
 *   Esta función envia un paquete UDP.
 *
 * PARÁMETROS:
 *   'dst_addr': Ip destino
 *   'port': Puerto utilizado para enviar
 *	 'payload': Puntero a los datos a enviar
 * 	 'payload_len': Tamaño de los datos a enviar
 *
 * VALOR DEVUELTO:
 * 		Devuelve 0 si el paquete ha sido creado, y enviado por a ipv4 correctamente
 *
 * ERRORES:
 *		La función devuelve err_code que sera !=0 si algo no ha ocurrido como lo esperado
 */
int udp_send(ipv4_addr_t dst_addr,uint16_t port, unsigned char * payload, int payload_len ){
	/*1. Declaramos y rellenamos el paquete UDP*/
	//Declaramos
	udp_dtg_t sent_pkt;
	//Rellenamos
	sent_pkt.port_src = htons(my_port);
	printf("Se enviará al puerto %d, desde el puerto %d\n", port, my_port);
	sent_pkt.port_dst = htons(port);
	sent_pkt.length = htons(UDP_HEADER_SIZE + payload_len);
	sent_pkt.checksum = htons(0); //lo ponemos a 0 porque asi no mira el pseudoheader
	memcpy(sent_pkt.udp_payload, payload, payload_len);

	//Lo mandamos a IPv4send
	int err_code = ipv4_send(dst_addr,UDP_IPv4_TYPE, (unsigned char *)&sent_pkt, UDP_HEADER_SIZE + payload_len );
	return err_code;
}


/*
 * int udp_recv(ipv4_addr_t src_addr, uint16_t port, unsigned char * buffer, int buffer_len, long int timeout )
 *
 * DESCRIPCIÓN:
 *   Esta función recibe un paquete UDP.
 *
 * PARÁMETROS:
 *   'src_addr': Ip source, nuestra IP
 *   'port': ARGUMENTO DE SALIDA : Develve el puerto de origen
 *	 'buffer': Puntero a al buffer donde se almacenan los datos recibidos
 * 	 'buffer_len': Tamaño de los datos recibidos
 *	 'timeout': timer que indica el tiempo que estaremos escuchando a recibir paquetes
 *
 * VALOR DEVUELTO:
 *   payload_len - UDP_HEADER_SIZE = tamaño de los datos de info sin el header de UDP
 *
 * ERRORES:
 *	 devuelve '-1' si hay un problema con la interfaz
 *   Devuelve '0' si no se ha recibido nada de payload
 */
int udp_recv(ipv4_addr_t src_addr, uint16_t *port, unsigned char * buffer, int buffer_len, long int timeout ){

		int payload_len = 0;
		udp_dtg_t * recv_packet = NULL;
		timerms_t timer;
		timerms_reset(&timer, timeout); //Ponemos el primer temporizador para que la escucha no sea eterna.

		do{ //Mientras que el puerto del que recibimos sea el deseado y el timer siga activo
			long int timeleft = timerms_left(&timer);//Calcula el tiempo restante del timer
			unsigned char udp_buffer[ETH_MTU];

			//Enviamos a IPv4
			payload_len = ipv4_recv(src_addr, UDP_IPv4_TYPE, udp_buffer,ETH_MTU, timeleft);
			//Comprobamos la carga
			if(payload_len==0) {
				return 0;// no se ha recibido nada
			}

		//Hacemos un casting de los datos recibidos a la estructura de una cabecera UDP
	  	recv_packet = (udp_dtg_t *) udp_buffer;

		}while(!(ntohs(recv_packet->port_dst) == my_port) );// para que no nos traguemos todos los paquetes de la red

		// "devolvemos" el puerto desde donde ha venido la información
		*port = ntohs(recv_packet->port_src);

		memcpy(buffer, recv_packet->udp_payload,payload_len - UDP_HEADER_SIZE); //Guardamos los datos recibidos

		return payload_len - UDP_HEADER_SIZE;
}

/*
* int get_rnd_port()
*
* DESCRIPCION
*   Genera un puerto aleatorio (un numero entre 1025 y MAX_RAND_PORT)
*
* VALOR DEVUELTO
*   Devuelve el valor generado
*/
int get_rnd_port(){
	/* Generar número aleatorio entre 0 y RAND_MAX */
	int dice = rand();
	/* Número entero aleatorio entre 1 y RAND_MAX Para el puerto de conexion*/
	int rnd_numb = 1025 + (int) (10.0 * dice / (MAX_RAND_PORT));
	return rnd_numb;
}
