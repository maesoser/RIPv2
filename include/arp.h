#ifndef _ARP_H
#define _ARP_H

#include <stdio.h>
#include <timerms.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>	// Time htons etc etc

#include "eth.h"
#include "ipv4.h"

/* int arp_resolve(eth_iface_t * iface,ipv4_addr_t ip_addr,mac_addr_t mac_addr);
 *
 * DESCRIPCIÓN:
 *   Esta función resuelve la query de arp basada en la IP que se le entregue
 *   nos devuelve la MAC asociada.
 *   Esta función usa funciones subordinadas para ejecutar algunas funciones
 *   estra como uso de caché o reintento de arp-request. 
 *   Primero se mira si esta en cache
 *    Si esta: se usa
 *    Si esta caducada: se usa arp unicast
 *    No esta: se hace broadcast
 *   Segundo: si dió error primero se hace un segundo intento en broadcast directamente
 *
 * PARÁMETROS:
 *   'iface': Puntero a la estructura del manejador del interfaz ethernet.
 *	 'ip_addr': Direccion IP por la que se pregunta.
 *	 'my_ipv4_addr: Nuestra direccion IP'
 *   'mac_addr': Direccion MAC que se encuentra.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si todo ha ido bien.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 */

int arp_resolve(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr);

/* int arp_rslv(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr,unsigned int timeout,unsigned int unicast);
  *
  * DESCRIPCIÓN:
  *   Esta función resuelve la query de arp basada en la IP que se le entregue
  *   nos devuelve la MAC asociada
  *
  * PARÁMETROS:
  *   'iface': Puntero a la estructura del manejador del interfaz ethernet.
  *	 'ip_addr': Direccion IP por la que se pregunta.
  *	 'my_ipv4_addr: Nuetsra direccion IP'
  *   'mac_addr': Direccion MAC que se encuentra.
  *   'timeout' : Tiempo limite para recibir el paquete
  *
  * VALOR DEVUELTO:
  *   La función devuelve '0' si todo ha ido bien.
  *
  * ERRORES:
  *   La función devuelve '-1' si se ha producido algún error.
  */

int arp_rslv(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr,unsigned int timeout,unsigned int unicast);

/* void cache_init();
 *
 * DESCRIPCIÓN:
 *   Esta función pone a cero el array de la cache
 */
void cache_init();


/* int cache_add(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Esta funcion añade una mac a un espacio vacío del array. 
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP que se quiere guardar.
 *   'mac_addr': Direccion MAC que se quiere guaradar.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si todo ha ido bien.
 *
 */
int cache_add(mac_addr_t mac_addr,ipv4_addr_t ip_addr);

/* int cache_resolve(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Resuelve una dirección MAC dando una dirección IP dentro de la caché ARP
 *   Esta funcion hace de arp_resolve pero dentro de la propia caché
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP por la que se pregunta.
 *   'mac_addr': Direccion MAC que se quiere.
 *
 * VALOR DEVUELTO:
 *   Si la encuentra y es actual, devuelve la dirección y 0
 *
 * ERRORES:
 *   Si la encuenta y es antigua, devuelve -1 y borra la direccion
 *   Si no la encuentra, devuelve -2
 */
int cache_resolve(mac_addr_t mac_addr,ipv4_addr_t ip_addr);

/* int cache_get_older();
 *
 * DESCRIPCIÓN:
 *   Recorre los indices de la caché para ver cual es el mas antiguo.
 *
 * VALOR DEVUELTO:
 *   Devuelve el indice de la entrada mas antigua
 */
int cache_get_older();

/* int cache_add_empty(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Si encuentra un slot vacío en la caché (viendo que el timestamp sea 0), 
 *   inserta la entrada arp_cache en ella. Sino devuelve fallo -1
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP que se quiere insertar.
 *   'mac_addr': Direccion MAC que se quiere insertar.
 *
 * VALOR DEVUELTO:
 *   Si todo es correct, devuelve 0.
 *
 * ERRORES:
 *   Si ha habido fallos, devuelve -1, por ejemplo falta de estacio
 */
int cache_add_empty(mac_addr_t mac_addr,ipv4_addr_t ip_addr);

/* int cache_show();
 *
 * DESCRIPCIÓN:
 *   Muestra los valores que hay en la cache ahora mismo
 *
 * VALOR DEVUELTO:
 *  Si todo es correcto, devuelve 0.
 */
int cache_show();

#endif /* _ARP_H */
