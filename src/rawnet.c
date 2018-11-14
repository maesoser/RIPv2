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
#include "rawnet.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/ethernet.h>     /* layer 2 protocols */
#include <netinet/in.h>

/* Requires GLIBC >= 2.1 */
#include <netpacket/packet.h>

/* Import error code variable from <errno.h> */
extern int errno;

#define RAWNET_ERROR_LENGTH 1024
static char rawnet_error[RAWNET_ERROR_LENGTH];

struct rawiface {
  char ifname[IF_NAMESIZE];
  int ifindex;
  int socket_fd;
};


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
rawiface_t * rawiface_open ( char* ifname )
{
  struct rawiface * iface;
  int err;

  /* Check 'ifname' parameter */
  if (ifname == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface name cannot be 'NULL'");
    return NULL;

  } else {
    int ifname_len = strlen(ifname);
    if (ifname_len >= IF_NAMESIZE) {
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "Invalid Raw Interface name \"%s\": Too long (%d >= %d)",
               ifname, ifname_len, IF_NAMESIZE);
      return NULL;
    }
  }

  /* Create a new struct rawiface  */
  iface = malloc(sizeof(struct rawiface));
  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot allocate memory for a new 'struct rawiface'");
    return NULL;
  }
  strcpy(iface->ifname, ifname);
  iface->ifindex = -1;
  iface->socket_fd = -1;

  /* Create a raw packet socket. See PACKET(7)
     - Needed now for ioctl() operations, bind() later to the appropriate
       interface.
     - Using ETH_P_ALL to support any L2 protocol. */
  int socket_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (socket_fd == -1) {
    char* err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot create a Raw Packet Socket: %s.\nNo superuser or CAP_NET_RAW capability?",
             err_str);
    free(iface);
    return NULL;
  }
  iface->socket_fd = socket_fd;

  /* Get interface index from interface name. See NETDEVICE(7)*/
  struct ifreq iface_ifreq;

  strcpy(iface_ifreq.ifr_name, iface->ifname);
  err = ioctl(iface->socket_fd, SIOCGIFINDEX, &iface_ifreq);
  if (err == -1) {
    char* err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot obtain index of Raw Interface \"%s\": %s",
             ifname, err_str);
    free(iface);
    return NULL;
  }
  iface->ifindex = iface_ifreq.ifr_ifindex;

  /* Bind packet socket to appropriate interface. See PACKET(7) */
  struct sockaddr_ll iface_sockaddr;

  iface_sockaddr.sll_family = PF_PACKET;
  iface_sockaddr.sll_protocol = htons(ETH_P_ALL);
  iface_sockaddr.sll_ifindex = iface->ifindex;

  err = bind(iface->socket_fd, (struct sockaddr*) &iface_sockaddr,
             sizeof(struct sockaddr_ll));
  if (err != 0) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot bind() Raw Packet Socket to \"%s\" interface: %s",
             iface->ifname, err_str);
    free(iface);
    return NULL;
  }

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return iface;
}


/* char* rawiface_getname ( rawiface_t * iface );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el nombre de la interfaz indicada.
 *
 * PARÁMETROS:
 *   'iface': Manejador de la interfaz de la que se quiere obtener su nombre.
 *   La interfaz debe haber sido inicializada con 'rawiface_open()'
 *   previamente.
 *
 * VALOR DEVUELTO:
 *   Cadena de texto con el nombre de la interfaz.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si la interfaz no ha sido inicializada
 *   correctamente.
 */
char* rawiface_getname ( rawiface_t * iface )
{
  char* ifname = NULL;

  if (iface != NULL) {
    ifname = iface->ifname;
  }

  return ifname;
}

/* int rawiface_getaddr ( rawiface_t * iface, char addr[] );
 *
 * DESCRIPCIÓN:
 *   Esta función permite obtener la dirección hardware de la interfaz
 *   indicada.
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
int rawiface_getaddr ( rawiface_t * iface, unsigned char addr[] )
{
  int halen;

  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface has not been initialized or it is 'NULL'");
    return -1;
  }

  struct sockaddr_ll iface_sockaddr;
  socklen_t iface_sockaddr_len = sizeof(struct sockaddr_ll);
  int err = getsockname
    (iface->socket_fd, (struct sockaddr *) &iface_sockaddr, &iface_sockaddr_len);
  if (err != 0) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot obtain Raw Interface HW address: %s", err_str);
    return -1;
  }
  halen = iface_sockaddr.sll_halen;
  memcpy(addr, iface_sockaddr.sll_addr, halen);

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return halen;
}

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
 *   El tamaño máximo de los datos de la trama que puede enviarse por esta
 *   interfaz.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 *   La descripción completa del error puede obtenerse a través de la función
 *   'rawnet_strerror()'.
 */
int rawiface_getmtu ( rawiface_t * iface )
{
  int mtu;

  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface has not been initialized or it is 'NULL'");
    return -1;
  }

  /* Get MTU. See NETDEVICE(7) */
  struct ifreq iface_ifreq;
  strcpy(iface_ifreq.ifr_name, iface->ifname);

  int err = ioctl(iface->socket_fd, SIOCGIFMTU, &iface_ifreq);
  if (err == -1) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot obtain Raw Interface MTU: %s", err_str);
    return -1;
  }
  mtu = iface_ifreq.ifr_mtu;

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return mtu;
}


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
int rawnet_send ( rawiface_t * iface, unsigned char * packet, int pkt_len )
{
  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface has not been initialized or it is 'NULL'");
    return -1;
  }

  int flags = 0;
  int err = send(iface->socket_fd, packet, pkt_len, flags);
  if (err == -1) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot send Raw Packet (%d bytes): %s", pkt_len, err_str);
    return -1;
  }

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return err;
}


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
( rawiface_t * iface, unsigned char buffer[], int buf_len, long int timeout )
{
  int packet_len;

  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface has not been initialized or it is 'NULL'");
    return -1;
  }

  /* First put socket in blocking/non-blocking mode according to timeout */

  int nonblock_mode = 0;
  if (timeout >= 0) {
    nonblock_mode = O_NONBLOCK;
  }

  int fd_mode = fcntl(iface->socket_fd, F_SETFL, nonblock_mode);
  if (fd_mode == -1) {
    if (nonblock_mode == 0) {
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "Cannot put Raw Packet Socket in blocking mode");
    } else {
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "Cannot put Raw Packet Socket in non-blocking mode");
    }
    return -1;
  }

  if (timeout > 0) {

    /* Use poll() to implement the timer */
    struct pollfd pollfd;
    pollfd.fd = iface->socket_fd;
    pollfd.events = POLLIN | POLLPRI;
    pollfd.revents = 0;
    int pollfd_num = 1;

    int err = poll(&pollfd, pollfd_num, timeout);
    if (err == -1) {
      char * err_str = strerror(errno);
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "Cannot wait for next raw frame: %s", err_str);
      return -1;

    } else if (err == 0) {

      /* Timeout has expired, return inmediately */
      return 0;
    }
  }

  /* Receive Raw Packet from the interface */
  int flags = MSG_TRUNC;
  packet_len = recv(iface->socket_fd, buffer, buf_len, flags);
  if (packet_len == -1) {

    if ((errno == EAGAIN) && (nonblock_mode == O_NONBLOCK)) {
      /* Timeout has expired */
      packet_len = 0;
    } else {
      char * err_str = strerror(errno);
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "Cannot receive Raw Packet: %s", err_str);
      return -1;
    }
  }

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return packet_len;
}


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
( rawiface_t * ifaces[], int ifnum, long int timeout )
{
  int iface_index;

  if (ifnum < 1) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "At least one Raw Interface must be specified (ifnum=%d)",
             ifnum);
    return -1;
  }

  /* Use poll() to listen all interfaces */
  int pollfds_num = ifnum;
  struct pollfd pollfds[pollfds_num];

  int i;
  for (i=0; i<pollfds_num; i++) {
    pollfds[i].fd = ifaces[i]->socket_fd;
    pollfds[i].events = POLLIN | POLLPRI;
    pollfds[i].revents = 0;
  }

  int err = poll(pollfds, pollfds_num, timeout);
  if (err < 0) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot wait for next Raw Packet: %s", err_str);
    return -1;

  } else if (err == 0) {

    /* Timeout has expired, return inmediately */
    return -2;

  } else {  /* err > 0 */

    /* Some interface has packets to read, find out which one */
    iface_index = -1;
    for (i=0; i<pollfds_num; i++) {
      short revents_i = pollfds[i].revents;
      if (((revents_i & POLLIN) == POLLIN) ||
          ((revents_i & POLLPRI) == POLLPRI)) {
        iface_index = i;
        break;
      }
    }

    if (iface_index == -1) {
      snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
               "No Raw Interface has any available data !?!");
      return -1;
    }
  }

  /* Clear error message */
  snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
           "No error, everything has gone OK");

  return iface_index;
}

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
int rawiface_close ( rawiface_t * iface )
{
  if (iface == NULL) {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Raw Interface has not been initialized or it is 'NULL'");
    return -1;
  }

  int err = close(iface->socket_fd);
  if (err != 0) {
    char * err_str = strerror(errno);
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "Cannot close Raw Packet Socket of Interface: %s", err_str);
  } else {
    snprintf(rawnet_error, RAWNET_ERROR_LENGTH,
             "No error, everything went OK");
  }

  free(iface);

  return err;
}

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
char* rawnet_strerror()
{
  return rawnet_error;
}
