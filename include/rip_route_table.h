#ifndef _RIP_ROUTE_TABLE_H
#define _RIP_ROUTE_TABLE_H

#include "rip.h"
#include "ipv4.h"
#include <stdio.h>
#include <timerms.h>

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32
#define RIPv2_ROUTE_TABLE_SIZE 256 /* Número de entradas máximo de la tabla de rutas IPv4 */

#define RIPv2_UPDATE 30000//30 secs
#define RIPv2_TIMEOUT 180000 //180 secs
#define RIPv2_GARBAGE_TIMEOUT 120000 // 120 secs

typedef struct ripv2_route_table ripv2_route_table_t;

typedef struct ripv2_route {
    ipv4_addr_t ip_addr;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t next_hop;
    uint32_t metric;
    timerms_t timer;
} ripv2_route_t;

/*// CREAR/ANADIR
ripv2_route_t * ripv2_route_create( ipv4_addr_t ip_addr, ipv4_addr_t mask, ipv4_addr_t nh,  uint32_t metric, long long int timeout);
ripv2_route_table_t * ripv2_route_table_create();
int ripv2_route_table_add ( ripv2_route_table_t * table, ripv2_route_t * route );

// ENCONTRAR/OBTENER
ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );
int ripv2_route_table_find( ripv2_route_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask );

// BORRAR
void ripv2_route_table_free ( ripv2_route_table_t * table );
void ripv2_route_free ( ripv2_route_t * route );
ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table, int index );

// MOSTRAR EN PANTALLA
void ripv2_route_print ( ripv2_route_t * route );
void ripv2_route_table_print ( ripv2_route_table_t * table );

// GUARDAR/RECUPERAR DE ARCHIVO
int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );
int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );

long int ripv2_get_min_timer(ripv2_route_table_t * table);
int ripv2_length(ripv2_route_table_t * table );
int ripv2_clear_table(ripv2_route_table_t * table );*/

// CREAR/ANADIR
/* ripv2_route_t * ripv2_route_create( ipv4_addr_t ip_addr, ipv4_addr_t mask, ipv4_addr_t nh,  uint32_t metric, long long int timeout);
 *
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta RIPv2 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ripv2_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 * ip_addr: ip a añadir
 * mask: mascara a añadir
 * nh: next hop a añadir
 * metric: metrica a añadir
 * timeout: timer a añadir
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
ripv2_route_t * ripv2_route_create( ipv4_addr_t ip_addr, ipv4_addr_t mask, ipv4_addr_t nh,  uint32_t metric, long long int timeout);
/* ripv2_route_table_t * ripv2_route_table_create();
 *
 * DESCRIPCIÓN:
 *   Esta función crea una tabla de rutas RipV2 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ripv2_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */
ripv2_route_table_t * ripv2_route_table_create();
/* int ripv2_route_table_add ( ripv2_route_table_t * table,
 *                            ripv2_route_t * route );
 * DESCRIPCIÓN:
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0,ripv2_route_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */
int ripv2_route_table_add ( ripv2_route_table_t * table, ripv2_route_t * route );

// ENCONTRAR/OBTENER
/* ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas especificada.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea obtener una ruta.
 *   'index': Índice de la ruta consultada. Debe tener un valor comprendido
 *            entre [0, ripv2_route_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no ha sido posible consultar la tabla de
 *   rutas, o no existe ninguna ruta en dicha posición.
 */
ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );
/* int ripv2_route_table_find ( ripv2_route_table_t * table, ipv4_addr_t subnet,
 *                                                         ipv4_addr_t mask );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el índice de la ruta para llegar a la subred
 *   especificada.
 *
 * PARÁMETROS:
 *    'table': Tabla de rutas en la que buscar la subred.
 *   'subnet': Dirección de la subred a buscar.
 *     'mask': Máscara de la subred a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la posición de la tabla de rutas donde se encuentra
 *   la ruta que apunta a la subred especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no se ha encontrado la ruta especificada o
 *   '-2' si no ha sido posible realizar la búsqueda.
 */
int ripv2_route_table_find( ripv2_route_table_t * table, ipv4_addr_t subnet, ipv4_addr_t mask );

// BORRAR
/* void ripv2_route_table_free ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la tabla de rutas
 *   especificada, incluyendo todas las rutas almacenadas en la misma,
 *   mediante la función 'ripv2_route_free()'.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas a borrar.
 */
void ripv2_route_table_free ( ripv2_route_table_t * table );
/* void ripv2_route_free ( ripv2_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ripv2_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea liberar.
 */
void ripv2_route_free ( ripv2_route_t * route );
/* ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table,
 *                                          int index );
 *
 * DESCRIPCIÓN:
 *   Esta función borra la ruta almacenada en la posición de la tabla de rutas
 *   especificada.
 *
 *   Esta función NO libera la memoria reservada para la ruta borrada. Para
 *   ello es necesario utilizar la función 'ripv2_route_free()' con la ruta
 *   devuelta.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea borrar una ruta.
 *   'index': Índice de la ruta a borrar. Debe tener un valor comprendido
 *            entre [0, ripv2_route_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta que estaba almacenada en la posición
 *   indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si la ruta no ha podido ser borrada, o no
 *   existía ninguna ruta en dicha posición.
 */
ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table, int index );

// MOSTRAR EN PANTALLA
/* void ripv2_route_print ( ripv2_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void ripv2_route_print ( ripv2_route_t * route );
/* void ripv2_route_table_print ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida estándar la tabla de rutas RIP
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 */
void ripv2_route_table_print ( ripv2_route_table_t * table );

// GUARDAR/RECUPERAR DE ARCHIVO
/* int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );
 *
 * DESCRIPCIÓN:
 *   Esta función almacena en el fichero especificado la tabla de rutas IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a almacenar.
 *   'filename': Nombre del fichero donde se desea almacenar la tabla de
 *               rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas almacenadas en el fichero de
 *   rutas, o '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir el
 *   fichero de rutas.
 */
int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );
/* int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función lee el fichero especificado y añade las rutas RIP
 *   estáticas leídas en la tabla de rutas indicada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero con rutas IPv4 que se desea leer.
 *      'table': Tabla de rutas donde añadir las rutas leidas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas leidas y añadidas en la tabla, o
 *   '0' si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al leer el
 *   fichero de rutas.
 */
int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );

/* timerms_t ripv2_get_min_timer(ripv2_route_table_t * table)
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el tiempo en ms del timer más cercano a espirar cercano a expirar
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizae.
 * VALOR DEVUELTO:
 *   min time: tiempo minimo encontrado
 */
long int ripv2_get_min_timer(ripv2_route_table_t * table);
/* int ripv2_length(ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función calcula la longitud de la tabla.
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizar.
 * DEVUELVE:
 *      el numero de rutas.
 */
int ripv2_length(ripv2_route_table_t * table );
/* int ripv2_clear_table(ripv2_route_table_t * table)
 *
 * DESCRIPCIÓN:
 *   Esta función limpia la tabla de entradas basura o pone a infinito entradas expiradas.
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizar.
 * DEVUELVE:
 *      1 Si alguna entrada está en garbage y debe ser anunciada.
 */
int ripv2_clear_table(ripv2_route_table_t * table );
#endif /* _RIP_ROUTE_TABLE_H */
