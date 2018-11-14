#include "eth.h"
#include "ipv4.h"
#include "udp.h"

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
		printf("Uso: %s <config_file>  <lookup_table>  <IP destino> <puerto> \n", myself);
		printf("\t<config_file>: Nombre de la interfaz Ethernet\n");
		printf("\t<lookup_table>: Tabla de reenvío \n");
		printf("\t<IP destino>: Dirección IP de destino\n");
		printf("\t<puerto>: Puerto de destino");

		exit(-1);
	}

	/* Procesar los argumentos de la línea de comandos */
	ipv4_addr_t destination_ip;
	if(ipv4_str_addr(argv[3],destination_ip)<0){
		fprintf(stderr, "%s: Dirección IP del servidor incorrecta: '%s'\n",
	    myself, argv[3]);
		exit(-1);
	}

	uint16_t port = atoi(argv[4]);

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

	/* Abriendo udp */
	err = udp_open(argv[1], argv[2],0);
	if(err < 0){
		fprintf(stderr, "%s: ERROR en udp_open\n", myself);
		exit(-1);
	}

	/* Enviar trama IP al Servidor */
	printf("Enviando %d bytes al Servidor UDP (%s):\n",payload_len, argv[3]);
	print_pkt(payload, payload_len, 0);

	err = udp_send(destination_ip, port, payload, payload_len);
	if (err < 0) {
		fprintf(stderr, "%s: ERROR en udp_sends()\n", myself);
		exit(-1);
	}

	// Recibir trama UDP del Servidor y procesar errores
	long int timeout = 2000;
	ipv4_addr_t src_addr;
	uint16_t src_port = 0;

	len = udp_recv(src_addr, &src_port, buffer, ETH_MTU, timeout );
	if (len < 0) {
		fprintf(stderr, "%s: ERROR en udp_recv()\n", myself);
	}
	if (len == 0) {
		fprintf(stderr, "%s: ERROR: No hay respuesta del Servidor UDP\n",myself);
	}
	if (len > 0) {
		char src_addr_str[IPv4_STR_MAX_LENGTH];
		ipv4_addr_str(src_addr, src_addr_str);
		printf("Recibidos %d bytes del Servidor UDP(%s)\n", len, src_addr_str);
		print_pkt(buffer, len, 0);
	}

	// Cerrar interfaz UDP
	printf("Cerrando interfaz UDP.\n");

	udp_close();

	return 0;
}
