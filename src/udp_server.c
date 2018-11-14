#include "eth.h"
#include "ipv4.h"
#include "udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

/* int main ( int argc, char * argv[] );
 *
 * DESCRIPCIÓN:
 *   Función principal del programa.
 *
 * PARÁMETROS:
 *   'argc': Número de parámetros pasados al programa por la línea de
 *           comandos, incluyendo el propio nombre del programa.
 *   'argv': Array con los parámetros pasado al programa por línea de
 *           comandos.
 *           El primer parámetro (argv[0]) es el nombre del programa.
 *
 * VALOR DEVUELTO:
 *   Código de salida del programa.
 */
int main ( int argc, char * argv[] )
{
  /* Mostrar mensaje de ayuda si el número de argumentos es incorrecto */
  char * myself = basename(argv[0]);
  if (argc != 4) {
    printf("Uso: %s <config file> <table file> <puerto>\n", myself);
    printf("       <config file>\n");
    printf("        <table file>\n");
    printf("        <port>\n");
    return -1;
  }

  uint16_t port = atoi(argv[3]);

  /* Abriendo udp */
  int err = udp_open(argv[1], argv[2], port);

  if(err < 0){
    fprintf(stderr, "%s: ERROR en udp_open\n", myself);
    return -1;
  }

  printf("Abriendo interfaz udp.\n");

  while(1) {

    /* Recibir trama Ethernet del Cliente */
    unsigned char buffer[ETH_MTU];
    long int timeout = -1;

    printf("Escuchando tramas udp (puerto %d) ...\n", port);
    ipv4_addr_t src_addr;
    uint16_t src_port = 0;
    int payload_len = udp_recv(src_addr, &src_port, buffer, ETH_MTU, timeout );
    if (payload_len == -1) {
      fprintf(stderr, "%s: ERROR en udp_recv()\n", myself);
      exit(-1);
    }

    char src_addr_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(src_addr, src_addr_str);

    printf("Recibidos %d bytes del Cliente udp (%s:%d):\n", payload_len, src_addr_str,src_port);
    print_pkt(buffer, payload_len, 0);

    /* Enviar la misma trama udp de vuelta al Cliente */
    printf("Enviando %d bytes al Cliente udp (%s:%d):\n",payload_len, src_addr_str,src_port);
    print_pkt(buffer, payload_len, 0);

    int len = udp_send(src_addr,src_port, buffer, payload_len);
    if (len == -1) {
      fprintf(stderr, "%s: ERROR en udp_send()\n", myself);
    }
  }

  /* Cerrar interfaz udp */
  printf("Cerrando interfaz udp.\n");

  udp_close();

  return 0;
}
