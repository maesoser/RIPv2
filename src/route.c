#include <stdio.h>
#include <timerms.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>	// Time htons etc etc

#include "arp.h"
#include "ipv4.h"
#include "ipv4_route_table.h"

int main(int argc,char *argv[]){
	
	ipv4_route_t miruta;
	ipv4_addr_t miaddr;
	int err = 0;
	int resultado = NULL;

	if(argc != 4){
		printf("\nUSO: %s <NET IP> <NET MASK> <IP>\n",argv[0]);
	}

	err = ipv4_str_addr(argv[1],  miruta.subnet_addr);
	if (err != 0) {
		printf("Dirección IP de la red incorrecta. \n");
		exit(-1);
	}

	err = ipv4_str_addr(argv[2],  miruta.subnet_mask);
	if (err != 0) {
		printf("Máscara de la red incorrecta. \n");
		exit(-1);
	}

	err = ipv4_str_addr(argv[3],  miaddr);
	if (err != 0) {
		printf("IP incorrecta. \n");
		exit(-1);
	}


	/*
	*   Si la dirección IPv4 pertenece a la subred de la ruta especificada, debe
	*   devolver un número positivo que indica la longitud del prefijo de
	*   subred. Esto es, el número de bits a uno de la máscara de subred.
	*   La función devuelve '-1' si la dirección IPv4 no pertenece a la subred
	*   apuntada por la ruta especificada.
	*/
	resultado = ipv4_route_lookup(&miruta, miaddr);
	if(resultado<0){
		printf("Esta IP no pertenece a esta subred\n");
	}
	else{
		printf("Tamaño de la máscara : %d\n",resultado);
	}
	return 0;
}
