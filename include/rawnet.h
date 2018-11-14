/* Copyright (C) 2010 Manuel Urueña <muruenya@it.uc3m.es>
 *
 * This file is part of librawnet.
 *
 * Librawnet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Librawnet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librawnet.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef _RAW_NET_H
#define _RAW_NET_H

typedef struct rawiface rawiface_t;

/* Tamaño máximo de una dirección hardware */
#define HW_ADDR_MAX_SIZE 8

/* rawiface_t * rawiface_open ( char* ifname );
 *
 * DESCRIPCIÓN:
 *   Esta función inicializa la interfaz hardware indicada para que pueda ser
 *   utilizada por las restantes funciones de la librería.
 *
 *   La memoria del manejador de interfaz devuelto debe ser liberada con la
 *   función 'rawiface_close()'.
 *
 * PARÁMETROS:
 *   'ifname' : Cadena de texto con el nombre de la interfaz hardware que se
 *              desea inicializar.
 *
 * VALOR DEVUELTO:
 *   Manejador de la interfaz hardware inicializada.
 *
 *   Dicho manejador es un puntero a una estructura opaca que no debe ser
 *   accedida directamente, en su lugar use las funciones de la librería.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
rawiface_t * rawiface_open ( char* ifname );


/* char* rawiface_getname ( rawiface_t * iface );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el nombre de la interfaz especificada.
 *
 * PARÁMETROS:
 *   'iface': Manejador de la interfaz de la que se quiere obtener su nombre.
 *            La interfaz debe haber sido inicializada con 'rawiface_open()'
 *            previamente.
 *
 * VALOR DEVUELTO:
 *   Cadena de texto con el nombre de la interfaz.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si la interfaz no ha sido inicializada
 *   correctamente.
 */
char* rawiface_getname ( rawiface_t * iface );


/* int rawiface_getaddr ( rawiface_t * iface, char addr[] );
 *
 * DESCRIPCIÓN:
 *   Esta función permite obtener la dirección hardware de la interfaz
 *   especificada.
 *
 * PARÁMETROS:
 *   'iface': Manejador de la interfaz de la que se quiere obtener su dirección.
 *            La interfaz debe haber sido inicializada con 'rawiface_open()'
 *            previamente.
 *    'addr': Array donde se copiará la dirección hardware de la interfaz.
 *            Las direcciones hardware pueden ocupar hasta 'HW_ADDR_MAX_SIZE'
 *            bytes.
 *
 * VALOR DEVUELTO:
 *   La longitud en bytes de la dirección hardware devuelta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawiface_getaddr ( rawiface_t * iface, unsigned char addr[] );


/* int rawiface_getmtu ( rawiface_t * iface );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve la Maximum Transmission Unit (MTU) permitida por
 *   esta interfaz.
 *
 * PARÁMETROS:
 *   'iface': Manejador de la interfaz de la que se quiere obtener su MTU.
 *            La interfaz debe haber sido inicializada con 'rawiface_open()'
 *            previamente.
 *
 * VALOR DEVUELTO:
 *   El tamaño máximo de la trama que puede enviarse por esta interfaz.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawiface_getmtu ( rawiface_t * iface );


/* int rawnet_send
 * ( rawiface_t * iface, unsigned char * packet, int flen );
 *
 * DESCRIPCIÓN:
 *   Esta función permite enviar un paquete a través de la interfaz indicada.
 *
 *
 * PARÁMETROS:
 *    'iface': Manejador de la interfaz por la que se quiere enviar el paquete.
 *             La interfaz debe haber sido inicializada con
 *             'rawiface_open()' previamente.
 *   'packet': Puntero al contenido del paquete que se quiere enviar. Debe
 *             incluir las cabeceras de nivel 2 y superiores.
 *  'pkt_len': Longitud en bytes del paquete a enviar.
 *
 * VALOR DEVUELTO:
 *   El número de bytes que han podido ser enviados.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawnet_send ( rawiface_t * iface, unsigned char * packet, int pkt_len );


/* int rawnet_recv
 * ( rawiface_t * iface, unsigned char buffer[], int buf_len, long int timeout );
 *
 * DESCRIPCIÓN:
 *   Esta función permite obtener el siguiente paquete recibido por la interfaz
 *   indicada. La operación puede esperar indefinidamente o un tiempo
 *   limitando dependiento del parámetro 'timeout'.
 *
 *   Esta función sólo permite recibir paquetes de una única interfaz. Si desea
 *   escuchar varias interfaces simultaneamente, utilice la función
 *   'rawnet_poll()'.
 *
 * PARÁMETROS:
 *    'iface': Manejador de la interfaz por la que se desea recibir un paquete.
 *             La interfaz debe haber sido inicializada con
 *             'rawiface_open()' previamente.
 *   'buffer': Array donde se almacenará el contenido del paquete recibido,
 *             incluyendo las cabeceras de nivel 2 y superiores.
 *  'buf_len': Longitud máxima del buffer en bytes. Si el tamaño del buffer es
 *             inferior al del paquete recibido, sólo se copiarán 'buf_len'
 *             bytes en el 'buffer' aunque la función  devolverá la longitud
 *             completa del paquete recibido.
 *  'timeout': Tiempo en milisegundos que debe esperarse a recibir un paquete
 *             antes de retornar. Un número negativo indicará que debe
 *             esperarse indefinidamente, mientras que con un '0' la función
 *             retornará inmediatamente, se haya recibido o no un paquete.
 *
 * VALOR DEVUELTO:
 *   La longitud en bytes del paquete recibido, o '0' si no se ha recibido
 *   ningún paquete porque ha expirado el temporizador.
 *   Tenga en cuenta que la longitud devuelta puede ser superior al número de
 *   bytes ('buf_len') copiados en el 'buffer'.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawnet_recv
( rawiface_t * iface, unsigned char buffer[], int buf_len, long int timeout );


/* int rawnet_poll
 * ( rawiface_t * ifaces[], int ifnum, long int timeout );
 *
 * DESCRIPCIÓN:
 *   Esta función permite esperar paquetes en múltiples interfaces
 *   simultaneamente. Cuando alguna de las interfaces indicadas reciba un
 *   paquete, la función devolverá la primera interfaz que tiene un paquete
 *   listo para ser recibido mediante la funcion 'rawnet_recv()'.
 *
 *   Esta operación puede escuchar de los interfaces indefinidamente o un
 *   tiempo limitado dependiento del parámetro 'timeout'.
 *
 * PARÁMETROS:
 *  'ifaces': Array con los manejadores de interfaces por los que se quiere
 *            recibir.
 *            Todos los interfaces deben haber sido inicializados con
 *            'rawiface_open()' previamente.
 *   'ifnum': Número de interfaces que aparecen en el array 'ifaces'.
 * 'timeout': Tiempo en milisegundos que debe esperarse a recibir un paquete
 *            antes de retornar. Un número negativo indicará que debe
 *            esperarse indefinidamente.
 *
 * VALOR DEVUELTO:
 *   El índice del primer interfaz [0, ifnum-1] que tiene un paquete listo para
 *   ser recibido o '-2' si ha expirado el temporizador.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawnet_poll
( rawiface_t * ifaces[], int ifnum, long int timeout );


/* int rawiface_close ( rawiface_t * iface );
 *
 * DESCRIPCIÓN:
 *   Esta función cierra la interfaz especificada y libera la memoria de su
 *   manejador.
 *
 * PARÁMETROS:
 *   'iface': Manejador de la interfaz que se desea cerrar.
 *
 * VALOR DEVUELTO:
 *   Devuelve 0 si la interfaz se ha cerrado correctamente.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawiface_close ( rawiface_t * iface );


/* char* rawnet_strerror();
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el último mensaje de error generado por alguna
 *   función de esta librería.
 *
 *   Hay funciones que no actualizan este mensaje de error, consulte la
 *   definición de la función.
 *
 * VALOR DEVUELTO:
 *   Cadena de texto con los detalles del último error producido por esta
 *   librería.
 */
char* rawnet_strerror();



#endif /* _RAW_NET_H */
