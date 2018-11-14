#ifndef _UDP_H
#define _UDP_H

#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "arp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Tamaño de un Header de UDP*/
#define UDP_HEADER_SIZE 8
/*1500 Eth - 20 IP - 8 header UDP = 1472*/
#define UDP_MAX_LENGTH 1472


/*
 * int udp_open(char *config, char *rtable,uint16_t port);
 *
 * DESCRIPCIÓN:
 *   Esta función abre una conexion UDP para enviar paquetes.
 *
 * PARÁMETROS:
 *   'config': Puntero al file donde esta guardada la configuracion
 *   'rtable': Puntero al file donde esta guardada la routing table
 *	 'port': Puerto random de conexion
 * VALOR DEVUELTO:
 *   El valor es '0' si la conexion udp ha sido abierta correctamente.
 *
 * ERRORES:
 *   La función devuelve err_code que sera !=0 si algo no ha ocurrido como lo esperado
 */
int udp_open(char *config, char *rtable,uint16_t port);

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
int udp_close();

/*
 * int udp_send(ipv4_addr_t dst_addr,uint16_t port, unsigned char * payload, int payload_len);
 *
 * DESCRIPCIÓN:
 *   Esta función envia un paquete UDP.
 *
 * PARÁMETROS:
 *   'dst_addr': Ip destino
 *   'port': Puerto utilizado
 *	 'payload': Puntero a los datos a enviar
 * 	 'payload_len': Tamaño de los datos a enviar
 *
 * VALOR DEVUELTO:
 * 		Devuelve 0 si el paquete ha sido creado, y enviado por a ipv4 correctamente
 *
 * ERRORES:
 *		La función devuelve err_code que sera !=0 si algo no ha ocurrido como lo esperado
 */
int udp_send(ipv4_addr_t dst_addr,uint16_t port, unsigned char * payload, int payload_len );

/*
 * int udp_recv(ipv4_addr_t src_addr, uint16_t port, unsigned char * buffer, int buffer_len, long int timeout )
 *
 * DESCRIPCIÓN:
 *   Esta función recibe un paquete UDP.
 *
 * PARÁMETROS:
 *   'src_addr': Ip source, nuestra IP
 *   'port': Puerto de donde ha VENIDO el paquete
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
int udp_recv(ipv4_addr_t src_addr, uint16_t *port, unsigned char * buffer, int buffer_len, long int timeout );

/*
* int get_rnd_port()
*
* DESCRIPCION
*   Genera un puerto aleatorio (un numero entre 1025 y MAX_RAND_PORT)
*
* VALOR DEVUELTO
*   Devuelve el valor generado
*/
int get_rnd_port();

#endif /* _UDP_H */
