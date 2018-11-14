#include "eth.h"
#include "ipv4.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#define DEFAULT_PAYLOAD_LENGTH 50

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
	if (argc != 5) {
		printf("Uso: %s <config_file>  <lookup_table>  <IP destino> <tipo>\n", myself);
		printf("		<config_file>: Nombre de la interfaz Ethernet\n");
		printf("		<lookup_table>: Tabla de reenvío \n");
		printf("		<IP destino>: Dirección IP de destino\n");
		printf("		<tipo> : Tipo de protocolo\n");
		exit(-1);
	}

	/* Procesar los argumentos de la línea de comandos */
	ipv4_addr_t destination_ip;
	if(ipv4_str_addr(argv[3],destination_ip)<0){
		fprintf(stderr, "%s: Dirección IP del servidor incorrecta: '%s'\n",
	    myself, argv[3]);
		exit(-1);
	}


	char* endptr;
	int type_int = (int) strtol(argv[4], &endptr, 0);
	if ((*endptr != '\0') || (type_int < 0) || (type_int > 0x0000FFFF)) {
    	fprintf(stderr, "%s: Tipo Ethernet incorrecto: '%s'\n",myself, argv[4]);
		exit(-1);
	}
	uint16_t type = (uint16_t) type_int;


	int len;
	unsigned char buffer[ETH_MTU];
	int payload_len = DEFAULT_PAYLOAD_LENGTH;
	int err;

	/* Generar payload */
	unsigned char payload[payload_len];
	int i;
	for (i=0; i<payload_len; i++) {
		payload[i] = (unsigned char) i;
	}

	/* Abriendo IP */
	err = ipv4_open(argv[1], argv[2]);
	if(err < 0){
		fprintf(stderr, "%s: ERROR en ipv4_open\n", myself);
		exit(-1);
	}

	/* Enviar trama IP al Servidor */
	printf("Enviando %d bytes al Servidor IP (%s):\n",payload_len, destination_ip);
	print_pkt(payload, payload_len, 0);

	err = ipv4_send(destination_ip,type, payload, payload_len);
	if (err < 0) {
		fprintf(stderr, "%s: ERROR en ipv4_send()\n", myself);
		exit(-1);
	}

	/* Recibir trama Ethernet del Servidor y procesar errores */
	long int timeout = 2000;
	ipv4_addr_t src_addr;

	len = ipv4_recv(src_addr, type, buffer, ETH_MTU, timeout );

	// Si ipv4_recv nos devuelve <0 es que ha habido algun error
	if (len < 0) {
		fprintf(stderr, "%s: ERROR en ipv4_recv()\n", myself);
	}

	// Si ipv4_recv nos devuelve 0 es que el timer ha expirado
	if (len == 0) {
		fprintf(stderr, "%s: ERROR: No hay respuesta del Servidor IP\n",myself);
	}

	if (len > 0) {
		char src_addr_str[IPv4_STR_MAX_LENGTH];
		ipv4_addr_str(src_addr, src_addr_str);
		printf("Recibidos %d bytes del Servidor IP (%s)\n", len, src_addr_str);
		print_pkt(buffer, len, 0);
	}

	/* Cerrar interfaz Ethernet */
	printf("Cerrando interfaz IP.\n");

	ipv4_close();

	return 0;
}
