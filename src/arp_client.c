#include <stdio.h>
#include <timerms.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>	// Time htons etc etc
#include "eth.h"
#include "ipv4.h"
#include "arp.h"

ipv4_addr_t ip_addr;
ipv4_addr_t my_ip_addr;
eth_iface_t *iface;
mac_addr_t mac_addr;

char mac_string[MAC_STR_LENGTH];
char ip_string[IPv4_STR_MAX_LENGTH];

int main(int argc,char *argv[]){
	// Si no hay suficientes argumentos
	if(argc != 4){
		printf("Escriba %s [IF] [IP] [MY_IP]\n", argv[0]);
		return -1;
	}
	// Si la IP está mal
	if(ipv4_str_addr ( argv[2], ip_addr )){
		printf("El argumento %s no es una IP válida\n",argv[1]);
		return -1;
	}

	if(ipv4_str_addr ( argv[3], my_ip_addr )){
		printf("El argumento %s no es una IP válida\n",argv[1]);
		return -1;
	}
	// Abre la interfaz
	iface = eth_open(argv[1]);

	//Hace ARP_resolve
	int result = arp_resolve(iface,ip_addr,my_ip_addr,mac_addr);

	//Si no recibimos cerramos la interfaz eth
	if(result != 0){
		eth_close(iface);
		return -1;
	}
	else{//Si recibimos nos quedamos con la info e imprimimos la info
		ipv4_addr_str(ip_addr , ip_string);
		mac_addr_str ( mac_addr , mac_string );
		printf("%s -> %s\n",ip_string, mac_string);
	}
	eth_close(iface);//CErramos eth.
	return 0;
}
