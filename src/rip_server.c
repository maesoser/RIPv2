
#include "udp.h"
#include "rip.h"
#include "rip_route_table.h"

ipv4_addr_t ip_addr;
int err;

unsigned char buffer[ETH_MTU]; // Reservamos más espacio para curarnos en salud.
ripv2_route_table_t *rip_table;
timerms_t update_timer;

unsigned char triggered_update = 0;

int get_entries_size(int size){
  //printf("%d bytes son %d entradas\n",size,(size -4)/20);
  if(size==0){
   return 0;
 }
  return (size -4)/20;
}

void free_and_exit(){
  printf("\nCerrando interfaz UDP.\n");
  udp_close();
  printf("Liberando Memoria.\n");
  ripv2_route_table_free( rip_table );
  exit(0);
}

void print_ripv2_msg(ripv2_msg_t *packet,int size){
  if(packet->command==RIP_REQUEST){
    printf("\tCOMMAND: REQUEST\n");
  }else {
    printf("\tCOMMAND: RESPONSE\n");
  }
  printf("\tRIP VER: %d\n", packet->version);
  int i = 0;
  printf("\tN\tTYPE\tTAG\tIP_ADDRESS\tSUBNET\t\t\tNEXT_HOP\tMETRIC\n");
  for(i=0;i<get_entries_size(size);i++){

      char sub_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].subnet_mask,sub_str);
      char ip_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].ip_addr,ip_str);
      char nh_str[IPv4_STR_MAX_LENGTH];
      ipv4_addr_str(packet->entries[i].next_hop,nh_str);

      printf("\t%d",i+1);
      printf("\t%d",ntohs(packet->entries[i].addr_id));
      printf("\t%d",ntohs(packet->entries[i].route_tag));
      printf("\t%s",ip_str);
      printf("\t%s",sub_str);
      printf("\t\t%s",nh_str);
      printf("\t\t%zu\n",ntohl(packet->entries[i].metric));

  }
}

/* ripv2_entry_t rip_get_entry(ripv2_route_t *rip_route) ;
 *
 * DESCRIPCIÓN:
 *   Esta función genera una ruta lista para ser integrada en un paquete reply.
 *   ATENCIÓN!! Devuelve el next_hop a 0.0.0.0 !!!!
 *
 * PARÁMETROS:
 *   'ripv2_route_t *rip_route'
 */

ripv2_entry_t rip_get_entry(ripv2_route_t *rip_route) {
  ripv2_entry_t entry;

  entry.addr_id = htons(0x02);
  entry.route_tag = htons(0x00);
  entry.metric = htonl(rip_route->metric);

  memcpy(entry.ip_addr, rip_route->ip_addr, IPv4_ADDR_SIZE);
  memcpy(entry.subnet_mask, rip_route->subnet_mask, IPv4_ADDR_SIZE);
  memcpy(entry.next_hop, IPv4_ZERO_ADDR, IPv4_ADDR_SIZE);

  return entry;
}

int send_table(ripv2_route_table_t *table, uint16_t src_port, ipv4_addr_t src_addr){

  ripv2_msg_t rip_req_pkt;
  bzero(&rip_req_pkt,RIPv2_PACKET_SIZE); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene
  rip_req_pkt.command = RIP_RESPONSE;
  rip_req_pkt.version = RIP_VERSION;

   // calculamos el total de entradas que quedan
  int total_entries = ripv2_length(table);
  if(total_entries>RIPv2_MAX_ENTRIES){
    total_entries=RIPv2_MAX_ENTRIES;
  }
  int i = 0;

  for (i= 0; i<total_entries; i++){
       ripv2_route_t *route_i = ripv2_route_table_get(table,i);
       if(route_i==NULL) {
        printf("ripv2_route_table_get da null");
        return -1;
        }
      ripv2_entry_t current_entry= rip_get_entry(route_i);
      if(current_entry.route_tag!=0) {
        printf("rip_get_entry da null");
        return -1;
      }

      memcpy(&(rip_req_pkt.entries[i]), &current_entry, RIPv2_ENTRY_SIZE);
  }

  print_ripv2_msg(&rip_req_pkt,total_entries*RIPv2_ENTRY_SIZE+RIPv2_HEADER_SIZE);

  int err = udp_send(src_addr, src_port, (uint8_t *)&rip_req_pkt, total_entries*RIPv2_ENTRY_SIZE + RIPv2_HEADER_SIZE);

  return err;
}

void send_request(ipv4_addr_t ip_addr){
  printf("Enviando REQUEST de toda la tabla\n");
  // Preparamos paquete RIP
  ripv2_msg_t rip_req_pkt;
  bzero(&rip_req_pkt,RIPv2_PACKET_SIZE); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene
  rip_req_pkt.command = RIP_REQUEST;
  rip_req_pkt.version = RIP_VERSION;
  rip_req_pkt.entries[0].metric = htonl(16);

  // Enviamos paquete RIP
  err = udp_send(ip_addr, RIPv2_UDP_PORT, (uint8_t *)&rip_req_pkt, RIPv2_HEADER_SIZE + RIPv2_ENTRY_SIZE);
  if (err < 0) {
      fprintf(stderr,"ERROR enviando REQUEST\n");
  }
}


int main(int argc,char *argv[]){

    signal (SIGINT, free_and_exit); // Registramos la señal para cerrar el servidor

    srand(time(NULL));  // To initialize the updated jitter

    rip_table =  ripv2_route_table_create();
    // Si el usuario ha metido algo, suponemos que es una tabla rip y la cargamos.
    if (argc == 2) {
      // Copiamos la IP y comprobamos consistencia
      err = ripv2_route_table_read ( argv[1], rip_table );
      if(err==0){
          fprintf(stderr, "ERROR: Archivo de rutas incorrecto '%s'\n",argv[1]);
          return -1;
      }
      else{
        printf("%d rutas importadas\n",err);
        ripv2_route_table_print(rip_table);

      }

    }

    if(argc>2){
      printf("Use: %s [routetable]\n", argv[0]);
      return -1;
    }

    // abrimos socket UDP
    err = udp_open(IP_CONFIG_FILE, ROUTE_CONFIG_FILE,RIPv2_UDP_PORT);//puerto 520 es el que usan los ruters rip
    if(err < 0){
        printf("ERROR  abriendo puerto\n");
        exit(-1);
    }

    send_request(IPv4_MULTICAST_ADDR);

    timerms_reset(&update_timer, RIPv2_UPDATE);
    while(1){//Escucha todos los paquetes

		  ipv4_addr_t src_addr;
		  uint16_t src_port = 0;
		  bzero(&buffer,ETH_MTU); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene

      //Calcula el siguiente momento en el que habrá que revisat la tabla
      long int timeout = ripv2_get_min_timer(rip_table);
      if(timerms_left(&update_timer) < timeout){
        timeout = timerms_left(&update_timer);
      }
		  int len = udp_recv(src_addr, &src_port, buffer, ETH_MTU, timeout );


      if (len < 0) {
			 fprintf(stderr, "ERROR en udp_recv()\n");
       exit(-1);
		  }

      if (len == 0) {
        ripv2_clear_table(rip_table);

        if (timerms_left(&update_timer) == 0) {

          if(ripv2_length(rip_table)!= 0){
            printf("Enviando Update:\n");
            send_table(rip_table,RIPv2_UDP_PORT,IPv4_MULTICAST_ADDR);
          }

          int jittered_time = RIPv2_UPDATE +rand()%15000;
          timerms_reset(&update_timer,jittered_time);
          printf("Proximo update en %d secs\n",jittered_time/1000);
        }
        ripv2_route_table_print(rip_table);
		  }

      if (len>=24) {//si el paquete lleva carga
        int i = 0;
        /*Declaramos variables para gestionar el paquete*/
			  char src_addr_str[IPv4_STR_MAX_LENGTH];  //Ip origen
			  ipv4_addr_str(src_addr, src_addr_str);
			  printf("Recibidos %d bytes del Servidor RIP : %s\n", len, src_addr_str);

			  ripv2_msg_t *rip_message; 	  // Puntero al paquete de reply
			  rip_message = (ripv2_msg_t *) buffer; //casting del buffer a la estructura de rip
        print_ripv2_msg(rip_message,len);

			  //print_ripv2_msg(rip_message,get_entries_size(len));         /*Imprime el paquete*/

        if(rip_message->command==RIP_REQUEST && (ripv2_length(rip_table)!=0) ){
	         printf("Respondiendo a un RIP REQUEST de toda la tabla\n");
          int entries_number = get_entries_size(len);
          //1. check si hay una unica entrada a merica 16
          if ((len == RIPv2_ENTRY_SIZE + RIPv2_HEADER_SIZE) && (ntohl(rip_message->entries[0].metric) == 16) && (ntohl(rip_message->entries[0].addr_id==0x00))){
              //enviar nuestra tabla entera desde el port 520 a la ip/port origen
            printf("Enviando toda la tabla\n");
            int err = send_table(rip_table,src_port,src_addr);
            if(err < 0){
                printf("ERROR contestando al request de tabla entera\n");
              exit(-1);
            }

          }
          else{           //2. Si se trata de rutas concretas ir rellenado el paquete y luego cambiarlo a tipo response
	    printf("Respondiendo a un RIP REQUEST : %d entries\n",entries_number);
            for(i=0;i<entries_number;i++){
            //checkear cada ruta si la tenemos y cambiar la metrica del paquete
              int index= ripv2_route_table_find(rip_table, rip_message->entries[i].ip_addr, rip_message->entries[i].subnet_mask);
              if (index != -1){
                ripv2_route_t *ruta = ripv2_route_table_get ( rip_table, index );
                rip_message->entries[i].metric= htonl(ruta->metric);
              }
              else{
                rip_message->entries[i].metric= htonl(16);
              }


            }
            //cambiar el command a RIP_RESPONSE
            rip_message->command=RIP_RESPONSE;
              err = udp_send(src_addr,src_port, (uint8_t *)rip_message, len );
              if(err < 0){
                printf("ERROR  contestando al request\n");
              exit(-1);
              }

          }
        }

        /*
        Es nueva?
          Si -> Añado
          No -|
            Es de mi padre?
              Si -> Actualizo
              No -|
                Tiene menor metrica?
                  Si -> Actualiza
                  No -> Nada
        */
        if(rip_message->command==RIP_RESPONSE){

          int i = 0;
          for(i = 0 ;i<get_entries_size(len); i++){

            int new_metric = ntohl(rip_message->entries[i].metric)+1;

            if (new_metric>=16){
              new_metric = 16;
            }

            long long int route_timeout = RIPv2_TIMEOUT;
            if(new_metric == 16) route_timeout = RIPv2_GARBAGE_TIMEOUT;

            int index= ripv2_route_table_find(rip_table, rip_message->entries[i].ip_addr, rip_message->entries[i].subnet_mask);
              if (index != -1){

                ripv2_route_t * rip_route = ripv2_route_table_get (rip_table, index);

                // SI VIENE DE PADRE
                if(memcmp(src_addr,rip_route->next_hop,IPv4_ADDR_SIZE)==0){
                  //printf("Proviene de root, actualizando\n");

                  ripv2_route_t *viejaruta = ripv2_route_table_remove ( rip_table, index );

                  if(viejaruta->metric == 16 && new_metric==16){
                    route_timeout = timerms_left(&viejaruta->timer);
                  }

                  if(new_metric!=viejaruta->metric){
                    triggered_update = 1;
                  }

                  ripv2_route_t *nuevaruta;

                  if(memcmp(rip_message->entries[i].next_hop,IPv4_ZERO_ADDR,IPv4_ADDR_SIZE)!=0){
                    nuevaruta = ripv2_route_create(
                      rip_message->entries[i].ip_addr,
                      rip_message->entries[i].subnet_mask,
                      rip_message->entries[i].next_hop,
                      new_metric,
                      route_timeout
                    );
                  }else{  //the next_hop ==0.0.0.0
                    nuevaruta = ripv2_route_create(
                      rip_message->entries[i].ip_addr,
                      rip_message->entries[i].subnet_mask,
                      src_addr,
                      new_metric,
                      route_timeout
                    );
                  }

                  int err = ripv2_route_table_add ( rip_table, nuevaruta );
                  if(err < 0){
                    printf("ERROR  añadiendo la ruta a la tabla de rip\n");
                  }

                }
                // SI NO VIENE DE PADRE
                else{
                  if(new_metric < rip_route->metric){
                    //printf("Mejor métrica, actualizando\n");

                    ripv2_route_table_remove ( rip_table, index );

                    ripv2_route_t *nuevaruta;

                    if(memcmp(rip_message->entries[i].next_hop,IPv4_ZERO_ADDR,IPv4_ADDR_SIZE)!=0){
                      nuevaruta = ripv2_route_create(
                        rip_message->entries[i].ip_addr,
                        rip_message->entries[i].subnet_mask,
                        rip_message->entries[i].next_hop,
                        new_metric,
                        route_timeout
                      );
                    }
                    else{
                      nuevaruta = ripv2_route_create(
                        rip_message->entries[i].ip_addr,
                        rip_message->entries[i].subnet_mask,
                        src_addr,
                        new_metric,
                        route_timeout
                      );
                    }

                    int err = ripv2_route_table_add ( rip_table, nuevaruta );
                    if(err < 0){
                      printf("ERROR  añadiendo la ruta a la tabla de rip\n");
                      exit(-1);
                    }
                    triggered_update = 1;

                  }
                  else{
                    //printf("La ruta no es mejor, idle\n");
                    //Refresco los timers
                  }
                }

              }
              // ES UNA NUEVA RUTA
              else{
                  //printf("La ruta es nueva, añadiendo\n");

                  ripv2_route_t *nuevaruta;

                  if(memcmp(rip_message->entries[i].next_hop,IPv4_ZERO_ADDR,IPv4_ADDR_SIZE)!=0){
                    nuevaruta = ripv2_route_create(
                      rip_message->entries[i].ip_addr,
                      rip_message->entries[i].subnet_mask,
                      rip_message->entries[i].next_hop,
                      new_metric,
                      route_timeout
                      );
                  }else{
                    nuevaruta = ripv2_route_create(
                      rip_message->entries[i].ip_addr,
                      rip_message->entries[i].subnet_mask,
                      src_addr,
                      new_metric,
                      route_timeout
                      );
                  }

                  int err = ripv2_route_table_add ( rip_table, nuevaruta );
                  if(err < 0){
                    printf("ERROR  añadiendo la ruta a la tabla de rip\n");
                  }

              }

          }
        }
        //1. validar el origen
          //2. actualizar la metrica "metric = MIN (metric + cost, infinity)"
          //3. check si ya estaba esa ruta expirando
          //4. añadir ruta: set addr, set metric, set net hop, initialize timeout, set route change flag, trigger if needed
          //4.1 si existia la ruta comparar y quedarnos la mejor (mirar RFC)
      }

      /*
        TRIGGERED UPDATES
      */
      if(triggered_update){
        printf("Enviando Triggered Update\n");
        triggered_update = 0;
        send_table(rip_table,RIPv2_UDP_PORT,IPv4_MULTICAST_ADDR);
        ripv2_route_table_print(rip_table);

      }

      
	}

  free_and_exit();

  return 0;
}
