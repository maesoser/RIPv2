#include "eth.h"
#include "ipv4.h"

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
    printf("Uso: %s <config file> <table file> <tipo>\n", myself);
    printf("       <config file>\n");
    printf("        <table file>\n");
    exit(-1);
  }


  /* El Tipo puede indicarse en hexadecimal (0x0800) o en decimal (2048) */
  char* type_str = argv[3];
  char* endptr;
  int type_int = (int) strtol(type_str, &endptr, 0);
  if ((*endptr != '\0') || (type_int < 0) || (type_int > 0x0000FFFF)) {
    fprintf(stderr, "%s: Tipo IP incorrecto: '%s'\n",
            myself, type_str);
    exit(-1);
  }
  uint8_t type = (uint8_t) type_int;

  /* Abriendo IP */
  int err = ipv4_open(argv[1], argv[2]);
  if(err < 0){
    fprintf(stderr, "%s: ERROR en ipv4_open\n", myself);
    exit(-1);
  }

  printf("Abriendo interfaz IP.\n");

  while(1) {

    /* Recibir trama Ethernet del Cliente */
    unsigned char buffer[ETH_MTU];
    ipv4_addr_t src_addr;
    long int timeout = -1;

    printf("Escuchando tramas IP (tipo=0x%04x) ...\n", type);
    int payload_len = ipv4_recv(src_addr, type, buffer, ETH_MTU, timeout );
    if (payload_len == -1) {
      fprintf(stderr, "%s: ERROR en ipv4_recv()\n", myself);
      exit(-1);
    }

    char src_addr_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(src_addr, src_addr_str);

    printf("Recibidos %d bytes del Cliente IP (%s):\n", payload_len, src_addr_str);
    print_pkt(buffer, payload_len, 0);

    /* Enviar la misma trama Ethernet de vuelta al Cliente */
    printf("Enviando %d bytes al Cliente IP (%s):\n",payload_len, src_addr_str);
    print_pkt(buffer, payload_len, 0);

    int len = ipv4_send(src_addr,type, buffer, payload_len);
    if (len == -1) {
      fprintf(stderr, "%s: ERROR en ipv4_send()\n", myself);
    }
  }

  /* Cerrar interfaz Ethernet */
  printf("Cerrando interfaz IP.\n");

  ipv4_close();

  return 0;
}
