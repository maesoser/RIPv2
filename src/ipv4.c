
#include "ipv4.h"
#include "ipv4_config.h"
#include "ipv4_route_table.h"
#include "arp.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*Tamaño del header de IPv4 sin contar las opciones*/
#define IPv4_HEADER_SIZE 20
/*Tamaño MTU en Ipv4== ETH_MTU-IPv4_HDR =1500-20=1480*/
#define IPv4_MTU 1480
/*Para rellenar el tipo de direccion software (proto_type) del paquete ARP*/
# define IPv4_ETH_TYPE 0x0800

/*Estructura de un paquete ipv4*/
typedef struct ipv4_packet {
	uint8_t version_ihl; //version+ihl= 8bits
	uint8_t type;
	uint16_t length;
	uint16_t id;
	uint16_t flags_offset; //flags+offset= 16 bits
	uint8_t ttl;
	uint8_t proto;         //protocol
	uint16_t checksum;
	ipv4_addr_t ip_addr_src;
	ipv4_addr_t ip_addr_dst;
	unsigned char ip_payload[IPv4_MTU];
} ipv4_pkt_t;

/*Como variables globales tenemos a addr y netmask para no tener que cargar el fichero de conf todo el rato*/
ipv4_addr_t my_ipv4_addr;
ipv4_addr_t netmask;
ipv4_route_table_t *table;
eth_iface_t *eth_if;

/* Dirección IPv4 a cero: "0.0.0.0" */
ipv4_addr_t IPv4_ZERO_ADDR = { 0, 0, 0, 0 };
ipv4_addr_t IPv4_MULTICAST_ADDR = { 224, 0, 0, 9 };
ipv4_addr_t broadcast_ip = {255,255,255,255};




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
int is_multicast(ipv4_addr_t ip_addr){
	if (ip_addr[0] >= 224 && ip_addr[0]<=239){
		return 1; //multicast
	}
	return 0;//unicast
}


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
void ipv4_addr_str ( ipv4_addr_t addr, char* str )
{
  if (str != NULL) {
    sprintf(str, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
  }
}


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
int ipv4_str_addr ( char* str, ipv4_addr_t addr )
{
  int err = -1;

  if (str != NULL) {
    unsigned int addr_int[IPv4_ADDR_SIZE];
    int len = sscanf(str, "%d.%d.%d.%d",
                     &addr_int[0], &addr_int[1],
                     &addr_int[2], &addr_int[3]);

    if (len == IPv4_ADDR_SIZE) {
      int i;
      for (i=0; i<IPv4_ADDR_SIZE; i++) {
        addr[i] = (unsigned char) addr_int[i];
      }

      err = 0;
    }
  }

  return err;
}


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
uint16_t ipv4_checksum ( unsigned char * data, int len )
{
  int i;
  uint16_t word16;
  unsigned int sum = 0;

  /* Make 16 bit words out of every two adjacent 8 bit words in the packet
   * and add them up */
  for (i=0; i<len; i=i+2) {
    word16 = ((data[i] << 8) & 0xFF00) + (data[i+1] & 0x00FF);
    sum = sum + (unsigned int) word16;
  }

  /* Take only 16 bits out of the 32 bit sum and add up the carries */
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  /* One's complement the result */
  sum = ~sum;

  return (uint16_t) sum;
}

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
int ipv4_open(char *config_file, char *table_file){

	char ifname[IFACE_NAME_MAX_LENGTH];

	/*1. Abrimos el fichero configuracion y lo cargamos en ifname, addr y netmask (siendo estas dos ultimas variables globales)*/
	// int ipv4_config_read( char* filename, char ifname[], ipv4_addr_t addr, ipv4_addr_t netmask );
	if(ipv4_config_read( config_file, ifname, my_ipv4_addr, netmask )<0) {
		printf("IPV4.C --> ipv4_open() --> ipv4_config_read(): No se ha podido abrir el archivo de configuracion IPv4\n");
		return -1;
	}

	table = ipv4_route_table_create(); // creamos una routing table

	/*2. Abrimos el fichero con la configuracion de la routing table y lo cargamos en table*/
	if(ipv4_route_table_read ( table_file, table )<0) {
		printf("IPV4.C --> ipv4_open() --> ipv4_route_table_read(): No se ha podido abrir el archivo de routing table IPv4\n");
		return -2;
	}

	/*3. Abrimos la interfaz*/
	eth_if = eth_open ( ifname );

	if(eth_if == NULL) {
		printf("IPV4.C --> ipv4_open() --> eth_open(): No se ha podido abrir la interfaz eth_if\n");
		return -3;
	}

	/*4.Fiheros cargados e interfaz abierta*/
	return 0;
}

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
int ipv4_close(){

	/*1. Cerramos ethernet*/
	if(eth_close(eth_if)<0){
		printf("IPV4.C --> ipv4_close() --> eth_close(): No se ha podido cerrar la interfaz eth_if\n");
		return -1;
	}

	/*2. Liberamos la memoria que ocupaba la tabla*/
	ipv4_route_table_free(table);

	/*3. Devolvemos 0 si se hacerrado la intefaz eth correctamente*/
	return 0;
}


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
int ipv4_send(ipv4_addr_t dst_addr,uint8_t protocol, unsigned char * payload, int payload_len ){

	ipv4_pkt_t send_pkt;

	/* Estructura del paquete IP (COMENTADO)
	uint8_t version_ihl ---> version+ihl= 8bits
		ihl=5 para eth
		version=4 ipv4
			01000101
	uint8_t type;---> QoS a 0
	uint8_t length;
	uint16_t id;
	uint16_t flags_offset; ---> flags+offset= 16 bits
	uint8_t ttl;
	uint8_t proto;       ---> protocol
	uint16_t checksum;
	ipv4_addr_t ip_addr_src;
	ipv4_addr_t ip_addr_dst;
	unsigned char ip_payload[IPv4_MTU];

	*/

	/*1. Rellenamos el paquete que vamos a mandar */
	//send_pkt.version_ihl = 0b01000101;
	send_pkt.version_ihl = 0x45;						//0x45
	send_pkt.type = 0;
	send_pkt.length = htons(payload_len + IPv4_HEADER_SIZE);	//RFC recomienda 576
	send_pkt.id = 0;
	//send_pkt.flags_offset = htons(0b0100000000000000); 			// 0 obligatorio, 1 dont fragment, 0 last fragment, 0's offset;
	send_pkt.flags_offset = htons(0x4000);
	if(is_multicast(dst_addr)){
		send_pkt.ttl = 1; 	     // Max linux TTL MULTICAST
	}else{
		send_pkt.ttl = 64; 	     // Max linux TTL UNICAST
	}
	send_pkt.proto = protocol;
	send_pkt.checksum = 0;										//Ponemos el checksum a 0, lo introducimos luego
	memcpy(send_pkt.ip_addr_src, my_ipv4_addr, IPv4_ADDR_SIZE); //Copiamos mi IP
	memcpy(send_pkt.ip_addr_dst, dst_addr, IPv4_ADDR_SIZE);		//Copiamos la IP detino
	memcpy(send_pkt.ip_payload, payload, payload_len);			//Copiamos la carga(datos)

	int checksum = ipv4_checksum((unsigned char*) &send_pkt ,IPv4_HEADER_SIZE);	//Hacemos el checksum del paquete
	send_pkt.checksum = htons(checksum);										//Introducimos el checksum

	mac_addr_t next_hop_mac;
	int err = ip_resolve(eth_if,my_ipv4_addr,dst_addr,next_hop_mac);
	if (err==-1) return -1;

	/*4. Mandamos el paquete por ethernet*/

	char ip_str[IPv4_STR_MAX_LENGTH];  //Ip origen
	ipv4_addr_str(dst_addr, ip_str);
	printf(" ENVIANDO A: %s\n",ip_str);

	int eth_res = eth_send ( eth_if, next_hop_mac, IPv4_ETH_TYPE, (unsigned char *)&send_pkt, payload_len + IPv4_HEADER_SIZE );

	if(eth_res <0){
		printf("IPV4.C --> ipv4_send() --> eth_send(): No se pede enviar paquete\n");
		return -2;
	}

	return 0;
}

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
int ipv4_recv(ipv4_addr_t src_addr, uint8_t protocol, unsigned char * buffer, int buffer_len, long int timeout ){

	//Declaramos variables
	int payload_len = 0;
	ipv4_pkt_t * recv_packet = NULL;
	mac_addr_t src_mac;
	timerms_t timer;

	int is_my_proto;
	int is_my_ip;

	if(eth_if == NULL){
		fprintf(stderr, "IPV4.C --> ipv4_recv(): ERROR iface == NULL\n");
		return -1;
	}

	timerms_reset(&timer, timeout); //Ponemos el primer temporizador para que la escucha no sea eterna.

	do{//Estará activo mientras el protocolo coincida con el deseado y la ip destino sea la nuestra

		long int timeleft = timerms_left(&timer);//Calcula el tiempo restante del timer
		unsigned char ip_buffer[ETH_MTU];

		//Nos ponemos a escuchar en ethernet
		payload_len = eth_recv(eth_if,src_mac,IPv4_ETH_TYPE, ip_buffer,ETH_MTU,timeleft);
		if(payload_len==0) {
			return 0;// no se ha recibido nada
		}

		//Casting de los datos recibidos a la estructura de un paquete IP
    	recv_packet = (ipv4_pkt_t *) ip_buffer;
		//printf("Recibo datagrama IP. Proto: %d\n", recv_packet->proto);
		is_my_proto = (recv_packet->proto==protocol);
		is_my_ip = (memcmp(recv_packet->ip_addr_dst, my_ipv4_addr, IPv4_ADDR_SIZE)==0);

		/*if(is_multicast(recv_packet->ip_addr_dst)){
			char ip_str[IPv4_STR_MAX_LENGTH];  //Ip origen
			ipv4_addr_str(recv_packet->ip_addr_dst, ip_str);
			printf("IP: MULTICAST RECEIVED: %s\n",ip_str);
		}*/

	}while(!(is_my_proto && (is_my_ip || is_multicast(recv_packet->ip_addr_dst))) );// para que no nos traguemos todos los paquetes de la red

	//Guardamos la Ip origen del paquete recibido
	memcpy(src_addr, recv_packet->ip_addr_src,IPv4_ADDR_SIZE);

	if(payload_len - IPv4_HEADER_SIZE > buffer_len){ //Si la payload es mayor que el buffer, copiamos lo que quepa
		memcpy(buffer, recv_packet->ip_payload,buffer_len);
	}
	else{											//Sino copiamos los datos
		memcpy(buffer, recv_packet->ip_payload,payload_len - IPv4_HEADER_SIZE);
	}

	return payload_len - IPv4_HEADER_SIZE; //Tamaño de los datos
}

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
int ip_resolve(eth_iface_t * eth_if, ipv4_addr_t src_ip_addr, ipv4_addr_t dst_ip_addr,mac_addr_t dst_mac_addr){

	/*CASO 1: es broadcast*/
	if(memcmp(dst_ip_addr,broadcast_ip,IPv4_ADDR_SIZE)==0){
     //printf("Broadcast addr detected\n");
     mac_addr_t broadcast_addr = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
     memcpy(dst_mac_addr,broadcast_addr,MAC_ADDR_SIZE);
     return 0;
   }

	/*CASO 2: es multicast*/
   if(dst_ip_addr[0]>=224 && dst_ip_addr[0]<=239){
     /*
     una MAC multicast es: 0x01,0x00,0x5E,0..[23 bytes de la IP multicast]
     mac = multicast_addr OR (ip_addr AND multicast_mask)
      OR -> Pone a 1 los bits a 1
      AND -> Pone a 0 los bist a 0
      ipv4_addr_t multicast_mask = {0x00,01111111,0xFF,0xFF};
     */
     mac_addr_t multicast_addr = {0x01,0x00,0x5E,0x00,0x00,0x00};
     //mac_addr[4] = ip_addr[2] & 0b01111111;
     memcpy(dst_mac_addr,multicast_addr,MAC_ADDR_SIZE);
     dst_mac_addr[3] = dst_ip_addr[1] & 0x7f;
     dst_mac_addr[4] = dst_ip_addr[2];
     dst_mac_addr[5] = dst_ip_addr[3];
     /*char mac_str[MAC_STR_LENGTH];
     mac_addr_str(mac_addr, mac_str);
     /printf("IP Multicast, MAC: %s\n",mac_str);*/
     return 0;
   }

   /*CASO 3. es unicast*/
   //buscamos la mejor ruta
	ipv4_route_t * prefered_route;
	prefered_route = ipv4_route_table_lookup ( table, dst_ip_addr );

	// Si la gateway es 0.0.0.0 -> Busca la IP destino
	if(memcmp(prefered_route->gateway_addr, IPv4_ZERO_ADDR, IPv4_ADDR_SIZE)==0){
		int arp_res = arp_resolve(eth_if,dst_ip_addr,src_ip_addr,dst_mac_addr);
		if(arp_res < 0){
			char addr_str[IPv4_STR_MAX_LENGTH];
			ipv4_addr_str(dst_ip_addr, addr_str);
			printf("IPV4.C --> ipv4_send() --> arp_resolve(): Imposible resolver la IP %s\n",addr_str);
			return -1;
		}
	}

	// Si existe una gateway valida, envia el paquete a su MAC. La gateway reenviará el paquete al PC destino
	else{
		int arp_res = arp_resolve(eth_if,prefered_route->gateway_addr,src_ip_addr,dst_mac_addr);
		if(arp_res < 0){
			char addr_str[IPv4_STR_MAX_LENGTH];
			ipv4_addr_str(prefered_route->gateway_addr, addr_str);
			printf("IPV4.C --> ipv4_send() --> arp_resolve(): Imposible resolver la IP %s\n",addr_str);
			return -1;
		}
	}
	
	return 0;

}
