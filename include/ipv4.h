#ifndef _IPv4_H
#define _IPv4_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Tamaño de una IPv4 (32 bits == 4 bytes). */
#define IPv4_ADDR_SIZE 4
/* Longitud en bytes de una cadena de texto que representa una dirección IP */
#define IPv4_STR_MAX_LENGTH 16

/* Definición del tipo para almacenar direcciones IP */
typedef unsigned char ipv4_addr_t [IPv4_ADDR_SIZE];

/* Dirección IPv4 a cero "0.0.0.0" */
extern ipv4_addr_t IPv4_ZERO_ADDR;
extern ipv4_addr_t IPv4_MULTICAST_ADDR;

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32

#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "arp.h"



/* int is_multicast(ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve un int en el que se indica si la ip es multicast
 *
 * PARÁMETROS:
 *   'ip_addr': La dirección IP que se quiere saber si es multicast
 *
 * VALORES DEVUELTOS:
 * 1 = es multicast
 * 0 = es unicast o broadcast
 */
int is_multicast(ipv4_addr_t ip_addr);
/* void ipv4_addr_str ( ipv4_addr_t addr, char* str );
 *
 * DESCRIPCIÓN:
 *   Esta función genera una cadena de texto que representa la dirección IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *   'addr': La dirección IP que se quiere representar textualente.
 *    'str': Memoria donde se desea almacenar la cadena de texto generada.
 *           Deben reservarse al menos 'IPv4_STR_MAX_LENGTH' bytes.
 */
void ipv4_addr_str ( ipv4_addr_t addr, char* str );


/* int ipv4_str_addr ( char* str, ipv4_addr_t addr );
 *
 * DESCRIPCIÓN:
 *   Esta función analiza una cadena de texto en busca de una dirección IPv4.
 *
 * PARÁMETROS:
 *    'str': La cadena de texto que se desea procesar.
 *   'addr': Memoria donde se almacena la dirección IPv4 encontrada.
 *
 * VALOR DEVUELTO:
 *   Se devuelve 0 si la cadena de texto representaba una dirección IPv4.
 *
 * ERRORES:
 *   La función devuelve -1 si la cadena de texto no representaba una
 *   dirección IPv4.
 */
int ipv4_str_addr ( char* str, ipv4_addr_t addr );


/*
 * uint16_t ipv4_checksum ( unsigned char * data, int len )
 *
 * DESCRIPCIÓN:
 *   Esta función calcula el checksum IP de los datos especificados.
 *
 * PARÁMETROS:
 *   'data': Puntero a los datos sobre los que se calcula el checksum.
 *    'len': Longitud en bytes de los datos.
 *
 * VALOR DEVUELTO:
 *   El valor del checksum calculado.
 */
uint16_t ipv4_checksum ( unsigned char * data, int len );

/*
 * int ipv4_open(char *config, char *rtable);
 *
 * DESCRIPCIÓN:
 *   Esta función abre una conexion IPv4 para enviar paquetes.
 *
 * PARÁMETROS:
 *   'config': Puntero al file donde esta guardada la configuracion
 *   'rtable': Puntero al file donde esta guardada la routing table
 * VALOR DEVUELTO:
 *   El valor es '0' si la conexion ipv4 ha sido abierta correctamente.
 *
 * ERRORES:
 *   La función devuelve -1 si no ha podido leer el archivo de configuracion
 *	 La función devuelve -2 si no ha podido leer el archivo de la routing table
 *	 La función devuelve -3 si no ha podido abrir la interfaz de eth
 */
int ipv4_open(char *config, char *rtable);

/*
 * int ipv4_close();
 *
 * DESCRIPCIÓN:
 *   Esta función cierra una conexion IPv4.
 *
 * VALOR DEVUELTO:
 *   El valor es '0' si la conexion ipv4 ha sido cerrada correctamente.
 *
 * ERRORES:
 *   La función devuelve -1 si no ha podido cerrar la interfaz eth
 */
int ipv4_close();

/*
 * int ipv4_send(ipv4_addr_t dst_addr,uint8_t protocol, unsigned char * payload, int payload_len );
 *
 * DESCRIPCIÓN:
 *   Esta función envia un paquete IPv4.
 *
 * PARÁMETROS:
 *   'dst_addr': Ip destino
 *   'protocol': Protocolo utilizado
 *	 'payload': Puntero a los datos a enviar
 * 	 'payload_len': Tamaño de los datos a enviar
 *
 * VALOR DEVUELTO:
 * 		Devuelve 0 si el paquete ha sido creado, arp_resolve y enviado por ethernet correctamente
 *
 *
 * ERRORES:
 *		Devuelve -1, si hay problemas con arp_resolve
 *		Devuelve -2, si hay problemas con eth_send
 */
int ipv4_send(ipv4_addr_t dst_addr,uint8_t protocol, unsigned char * payload, int payload_len );

/*
 * int ipv4_recv(ipv4_addr_t src_addr, uint8_t protocol, unsigned char * buffer, int buffer_len, long int timeout );
 *
 * DESCRIPCIÓN:
 *   Esta función recibe un paquete IPv4.
 *
 * PARÁMETROS:
 *   'src_addr': Ip source, nuestra IP
 *   'protocol': Protocolo utilizado
 *	 'buffer': Puntero a al buffer donde se almacenan los datos recibidos
 * 	 'buffer_len': Tamaño de los datos recibidos
 *	 'timeout': timer que indica el tiempo que estaremos escuchando a recibir paquetes
 *
 * VALOR DEVUELTO:
 *   payload_len - IPv4_HEADER_SIZE = tamaño de los datos de info sin el header de IPv4
 *
 * ERRORES:
 *	 devuelve '-1' si hay un problema con la interfaz
 */
int ipv4_recv(ipv4_addr_t src_addr, uint8_t protocol, unsigned char * buffer, int buffer_len, long int timeout );

/*
 * int ip_resolve(eth_iface_t * eth_if, ipv4_addr_t src_ip_addr, ipv4_addr_t dst_ip_addr,mac_addr_t dst_mac_addr){
 *
 * DESCRIPCIÓN:
 *   Esta función se encarga del routing y de encontrar la mac asociada a la IP del siguiente salto.
 *	 Distingue si es una IP multicast o no:
 *		Si es multicast: Calcula la MAC multicast
 *		Si no es multicast: Busca en la tabla de rutas
 *			Si el gateway es 0.0.0.0 -> Envia a la MAC de destino.
 *			Si el gateway no es 0.0.0.0 -> Envia a la MAC del siguiente salto
 *
 * PARÁMETROS:
 *   'eth_if': Interfaz que usamos
 *   'src_ip_addr': Nuetra IP
 *	 'dst_ip_addr': IP a donde queremos enviar el datagrama
 * 	 'mac_addr_t': MAC que queremos averiguar
 *
 * VALOR DEVUELTO:
 *   payload_len - IPv4_HEADER_SIZE = tamaño de los datos de info sin el header de IPv4
 *
 * ERRORES:
 *	 devuelve '-1' si ARP no ha sido capaz de encontrar dicha IP
 */

int ip_resolve(eth_iface_t * eth_if, ipv4_addr_t src_ip_addr, ipv4_addr_t dst_ip_addr,mac_addr_t dst_mac_addr);

#endif /* _IPv4_H */
