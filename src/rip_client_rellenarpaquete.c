/*
Basicamente
    Definimos un paquete arp.
    Lo rellenamos para hacer un request. Lo enviamos a la IP que le pasemos como argumento de linea de comandos.
    Capturamos el paquete de vuelta, lo desensamblamos y lo mostramos.
    CLAVAO a ARP (en el modo de operacion, bueno, ahora estamos en UDP pero es basicamente lo mismo)
*/

#include "udp.h"
#include "rip.h"
#include "rip_route_table.h"
#include <unistd.h>
int get_entries_size(int size){
  printf("%d bytes son %d entradas\n",size,(size -4)/20);
  return (size -4)/20;

}

void print_ripv2_msg(ripv2_msg_t *packet,int size){
  if(packet->command==RIP_REQUEST) printf("COMMAND: REQUEST\n");
  else printf("COMMAND: REPLY\n");
  printf("RIP VER: %d\n", packet->version);
  int i = 0;
  printf("N\tTYPE\tTAG\tIP_ADDRESS\tSUBNET\t\t\tNEXT_HOP\tMETRIC\n");
  for(i=0;i<size;i++){

      char sub_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].subnet_mask,sub_str);
      char ip_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].ip_addr,ip_str);
      char nh_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].next_hop,nh_str);

      printf("%d\t%d\t%d\t%s\t%s\t\t%s\t\t%zu\n",i+1,ntohs(packet->entries[i].addr_id),ntohs(packet->entries[i].route_tag),ip_str,sub_str,nh_str,ntohl(packet->entries[i].metric));

  }
}

ipv4_addr_t ip_addr;
int err;
unsigned char buffer[ETH_MTU];

int main(int argc,char *argv[]){

    // Si hay != 1 argumento, mostramos esta ayuda
    if (argc != 3) {
        printf("Use: %s <ip>, <Request=2 o Response=1>\n", argv[0]);
        return -1;
    }
    
    // Copiamos la IP y comprobamos consistencia
    if(ipv4_str_addr(argv[1],ip_addr)<0){
        fprintf(stderr, "ERROR: Dirección IP del servidor incorrecta: '%s'\n",argv[1]);
        return -1;
    }

    // abrimos socket UDP
    err = udp_open(IP_CONFIG_FILE, ROUTE_CONFIG_FILE,0);
    if(err < 0){
        printf("ERROR  abriendo puerto\n");
        exit(-1);
    }

    // Preparamos paquete RIP
    int tipo_de_paquete = atoi(argv[2]);
    printf("Requesting to %s\n a message of type %d\n 1=request, 2=response",argv[1], tipo_de_paquete);
    ripv2_msg_t rip_req_pkt;
    bzero(&rip_req_pkt,RIPv2_PACKET_SIZE); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene
    
    if (tipo_de_paquete==1){ //con response enviamos nuestra tabla

    rip_req_pkt.command = RIP_RESPONSE;
    rip_req_pkt.version = RIP_VERSION;
    ipv4_addr_t destination_ip;
    ipv4_addr_t mask_ip;


    rip_req_pkt.entries[0].metric = htonl(3);
    ipv4_str_addr("168.23.54.123",destination_ip);
    memcpy(rip_req_pkt.entries[0].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[0].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[0].addr_id=htons(0x02);

    rip_req_pkt.entries[1].metric = htonl(7);
    ipv4_str_addr("168.23.54.127",destination_ip);
    memcpy(rip_req_pkt.entries[1].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[1].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[1].addr_id=htons(0x02);

    rip_req_pkt.entries[2].metric = htonl(13);
    ipv4_str_addr("168.23.54.128",destination_ip);
    memcpy(rip_req_pkt.entries[2].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[2].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[2].addr_id=htons(0x02);




    // Enviamos paquete RIP
    err = udp_send(ip_addr, RIPv2_UDP_PORT, (uint8_t *)&rip_req_pkt, RIPv2_HEADER_SIZE + RIPv2_ENTRY_SIZE*3);
    if (err < 0) {
        fprintf(stderr,"ERROR enviando\n");
        exit(-1);
    }
    sleep(3);


    rip_req_pkt.entries[0].metric = htonl(16);
    ipv4_str_addr("168.23.54.123",destination_ip);
    memcpy(rip_req_pkt.entries[0].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[0].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[0].addr_id=htons(0x02);


    rip_req_pkt.entries[1].metric = htonl(4);
    ipv4_str_addr("168.23.54.127",destination_ip);
    memcpy(rip_req_pkt.entries[1].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[1].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[1].addr_id=htons(0x02);


    rip_req_pkt.entries[2].metric = htonl(5);
    ipv4_str_addr("168.23.54.128",destination_ip);
    memcpy(rip_req_pkt.entries[2].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[2].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[2].addr_id=htons(0x02);




    // Enviamos paquete RIP
    err = udp_send(ip_addr, RIPv2_UDP_PORT, (uint8_t *)&rip_req_pkt, RIPv2_HEADER_SIZE + RIPv2_ENTRY_SIZE*3);
    if (err < 0) {
        fprintf(stderr,"ERROR enviando\n");
        exit(-1);
    }


  // Cerrar interfaz UDP
  printf("Cerrando interfaz UDP.\n");

  udp_close();


    return 0;

    }else if(tipo_de_paquete==2){//ficticio le estoy enviando mi tabla


    rip_req_pkt.command = RIP_REQUEST;
    rip_req_pkt.version = RIP_VERSION;
    ipv4_addr_t destination_ip;
    ipv4_addr_t mask_ip;

    rip_req_pkt.entries[0].metric = htonl(16);
    ipv4_str_addr("168.23.54.123",destination_ip);
    memcpy(rip_req_pkt.entries[0].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[0].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[0].addr_id=htons(0x02);


    rip_req_pkt.entries[1].metric = htonl(16);
    ipv4_str_addr("168.23.54.127",destination_ip);
    memcpy(rip_req_pkt.entries[1].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[1].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[1].addr_id=htons(0x02);

    rip_req_pkt.entries[2].metric = htonl(16);
    ipv4_str_addr("168.23.54.128",destination_ip);
    memcpy(rip_req_pkt.entries[2].ip_addr,destination_ip, IPv4_ADDR_SIZE);
    ipv4_str_addr("255.255.255.0",mask_ip);
    memcpy(rip_req_pkt.entries[2].subnet_mask,mask_ip, IPv4_ADDR_SIZE);
    rip_req_pkt.entries[2].addr_id=htons(0x02);

    // Enviamos paquete RIP
    err = udp_send(ip_addr, RIPv2_UDP_PORT, (uint8_t *)&rip_req_pkt, RIPv2_HEADER_SIZE + RIPv2_ENTRY_SIZE*3);
    if (err < 0) {
        fprintf(stderr,"ERROR enviando\n");
        exit(-1);
    }

    ipv4_addr_t src_addr;
    uint16_t src_port = 0;
    bzero(&buffer,ETH_MTU); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene

    int len = udp_recv(src_addr, &src_port, buffer, ETH_MTU, UDP_RCV_TIMEOUT );
  if (len < 0) {
    fprintf(stderr, "ERROR en udp_recv()\n");
  }
  if (len == 0) {
    fprintf(stderr, "ERROR: No hay respuesta del Servidor RIP\n");
  }
  if (len > 0) {
    char src_addr_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(src_addr, src_addr_str);
    printf("Recibidos %d bytes del Servidor RIP(%s)\n", len, src_addr_str);
    //print_pkt(buffer, len, 0);
    ripv2_msg_t *reply_packet;    // Puntero al paquete de reply
    reply_packet = (ripv2_msg_t *) buffer;
    print_ripv2_msg(reply_packet,get_entries_size(len));

  }

  // Cerrar interfaz UDP
  printf("Cerrando interfaz UDP.\n");

  udp_close();


    return 0;

    }else{
      printf("Tipo de paquete seleccionado no es correcto \n");
    }

    
return 0;
}
