#include "arp.h"

/* Tamaño de una IPv4 (32 bits == 4 bytes). */
//#define IPv4_ADDR_SIZE 4 // define DUPLICADO por eso esta comentado

/*Para rellenar el tipo de direccion hardware (hw_type) del paquete ARP*/
#define ETHER_ETH_TYPE 0x0001
/*Para rellenar el tipo de direccion software (proto_type) del paquete ARP*/
#define IPv4_ETH_TYPE 0x0800

/* Valor del campo Type en un paquete ARP para que sea ethernet */
#define ARP_ETH_TYPE 0x0806
/* Valor del campo Code (op_code) en un paquete ARP pata que sea request*/
#define ARP_REQ_CODE 0x0001
/* Valor del campo Code (op_code) en un paquete ARP pata que sea reply*/
#define ARP_REP_CODE 0x0002
/* Tamaño de un paquete ARP */
#define ARP_MSG_SIZE 28

#define FIRST_ATTEMPT_TIMEOUT 2000  //Primer intento, 2 segundos  
#define SECOND_ATTEMPT_TIMEOUT 3000  //Segundo itento, 3 segundos
#define UNICAST_REQUEST 1
#define BROADCAST_REQUEST 0

/* Numero máximo de entradas ARP*/
#define CACHE_LENGTH 2
/* Tiempo de vida de  una entrada en la cache ARP */
#define CACHE_TTL 5  //5 segundos dura una entrada en la cache, si se reusa se pone el timer a 0

/* estructura de una entrada de la cache ARP */
typedef struct arp_entry{
  ipv4_addr_t ip_addr;
  mac_addr_t mac_addr;
  time_t last_time;            //Para indicar cuanto tiempo lleva la entrada en la tabla, es un TIME STAM NO UN TIMER
} arp_entry_t;

/* Esta es la estructura de un paquete ARP */
typedef struct arp_pkt_t{
	uint16_t hw_type;			       // Tipo de dirección de hardware
	uint16_t proto_type;		     // Tipo de dirección de software
	uint8_t hw_size;			       // Longitud de la dirección de hardware
	uint8_t proto_size;			     // Longitud de la direccion de software
	uint16_t op_code;			       //  Codigo de la operacion (Request o reply)
	mac_addr_t src_hw_addr; 	   // Dirección de hardware de origen
	ipv4_addr_t src_proto_addr;	 // Direccion IP de origen
	mac_addr_t dst_hw_addr;		   // Dirección de hardware de destino (La MAC que deseamos saber)
	ipv4_addr_t dst_proto_addr;  // Direccion IP de destino (por la que preguntamos la MAC)
} arp_pkt ;


/*Variable globales*/
int recv_bytes;                         // Bytes recibidos
mac_addr_t src_mac;                     // MAC desde donde se envía la respuesta
unsigned char inbuffer[ETH_MTU];        // Buffer de entrada
timerms_t timer;                        //Definicion del timer de espera a respuesta
unsigned char cache_initialized = 0;
arp_entry_t cache_table[CACHE_LENGTH];  // Inicializacion de la Cache ARP (array de estructuras)

/* int arp_resolve(eth_iface_t * iface,ipv4_addr_t ip_addr,mac_addr_t mac_addr);
 *
 * DESCRIPCIÓN:
 *   Esta función resuelve la query de arp basada en la IP que se le entregue
 *   nos devuelve la MAC asociada.
 *   Esta función usa funciones subordinadas para ejecutar algunas funciones
 *   estra como uso de caché o reintento de arp-request. 
 *   Primero se mira si esta en cache
 *    Si esta: se usa
 *    Si esta caducada: se usa arp unicast
 *    No esta: se hace broadcast
 *   Segundo: si dió error primero se hace un segundo intento en broadcast directamente
 *
 * PARÁMETROS:
 *   'iface': Puntero a la estructura del manejador del interfaz ethernet.
 *	 'ip_addr': Direccion IP por la que se pregunta.
 *	 'my_ipv4_addr: Nuestra direccion IP'
 *   'mac_addr': Direccion MAC que se encuentra.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si todo ha ido bien.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error.
 */

 int arp_resolve(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr){
   int result = 0;

   /*0. Inicializacion de la Cache ARP*/
   cache_init();                 // Ponemos la cache a 0
   cache_show();                 // Mostramos la cache (DEBUG)

   /*1. Comprobamos si ya existe una entrada valida en la cache ARP*/
   int cache = cache_resolve(mac_addr,ip_addr);

   if(cache==0){
     return 0; // Si se ha encontrado una entrada válida en la cache, salimos. Porque ya tenemos la MAC asociada
   }
   if(cache==-1){ //-1 = estaba la entrada en la cache pero era antigua. Hacemos un primer intento por unicast
     result = arp_rslv(iface,ip_addr,my_ipv4_addr,mac_addr,FIRST_ATTEMPT_TIMEOUT,UNICAST_REQUEST);
   }
   if(cache==-2){ //-2= no estaba la entrada en la cache. Hacemos un segundo intento por broadcast
     result = arp_rslv(iface,ip_addr,my_ipv4_addr,mac_addr,FIRST_ATTEMPT_TIMEOUT,BROADCAST_REQUEST);
   }


   if(result != 0) { //Si con el primer timer da fallo se intenta una segunda vez por broadcast directamente
     printf("eth_send: Reintentando ARP REQUEST\n");
     result = arp_rslv(iface,ip_addr,my_ipv4_addr,mac_addr,SECOND_ATTEMPT_TIMEOUT,BROADCAST_REQUEST);
   }

   return result;

 }

 /* int arp_rslv(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr,unsigned int timeout,unsigned int unicast);
  *
  * DESCRIPCIÓN:
  *   Esta función resuelve la query de arp basada en la IP que se le entregue
  *   nos devuelve la MAC asociada
  *
  * PARÁMETROS:
  *   'iface': Puntero a la estructura del manejador del interfaz ethernet.
  *	 'ip_addr': Direccion IP por la que se pregunta.
  *	 'my_ipv4_addr: Nuetsra direccion IP'
  *   'mac_addr': Direccion MAC que se encuentra.
  *   'timeout' : Tiempo limite para recibir el paquete
  *
  * VALOR DEVUELTO:
  *   La función devuelve '0' si todo ha ido bien.
  *
  * ERRORES:
  *   La función devuelve '-1' si se ha producido algún error.
  */

  int arp_rslv(eth_iface_t * iface,ipv4_addr_t ip_addr,ipv4_addr_t my_ipv4_addr,mac_addr_t mac_addr,unsigned int timeout,unsigned int unicast){
    
    /*2. Declaramos y rellenamos el paquete ARP REQUEST*/
    int err = 0;
    //Declaramos el paquete
  	arp_pkt req_packet; 	    //Nombre de la estructura del arp paket que vamos a usar para la request (req_packet)
  	arp_pkt *reply_packet; 	  // Puntero al paquete de reply

  	//Rellenamos el paquete de request
  	req_packet.hw_type = htons(ETHER_ETH_TYPE);
  	req_packet.proto_type = htons(IPv4_ETH_TYPE);
  	req_packet.hw_size= MAC_ADDR_SIZE;
  	req_packet.proto_size = IPv4_ADDR_SIZE;
  	req_packet.op_code = htons(ARP_REQ_CODE);
  	bzero(req_packet.dst_hw_addr,MAC_ADDR_SIZE); //La MAC de la IP por la que preguntamos ha de ir a 0 para que se rellene
  	memcpy(req_packet.dst_proto_addr,ip_addr,IPv4_ADDR_SIZE); // Ip que quiero buscar la copiamos en el espacio asignado para IP dst
  	mac_addr_t mac_addr_src; //Nuestra MAC
  	eth_getaddr(iface,mac_addr_src); //Esta funcion nos copia en iface la interface asociada a la mac nuestra que usaremos
  	memcpy(req_packet.src_hw_addr,mac_addr_src,MAC_ADDR_SIZE); // Nuestra MAC la copiamos en el apartado de MAC source del paquete ARP
  	memcpy(req_packet.src_proto_addr,my_ipv4_addr,IPv4_ADDR_SIZE); // Nuestra Ip la copiamos en el espacio asignado para IP dst


    /*3. Enviamos el paquete ARP REQUEST*/

      //Enviamos el paquete con eth_send por broadcast/unicast
      if(unicast){// el bit de unicast lo heredamos de arp_resolve
        err = eth_send(iface,mac_addr,ARP_ETH_TYPE,(unsigned char *) &req_packet, ARP_MSG_SIZE);
      }
      else{//si no es unicast es broadcast
        err = eth_send(iface,MAC_BCAST_ADDR,ARP_ETH_TYPE,(unsigned char *) &req_packet, ARP_MSG_SIZE);
      }

    	if(err < 0) {
    		return -1;
    		printf("eth_send: No se ha podido enviar el paquete\n");
    	}

      timerms_reset(&timer, timeout);   //Ponemos el primer temporizador para que la escucha no sea eterna. Lo ponemos al valor de FIRST o SECOND timer

      do{ //Este do-while dejará de actuar cuando ip_addr sea == que la IP source del paquete reply

        /*4. Ponemos el primer timer de escucha*/
        long int timeleft = timerms_left(&timer);                                    //Calcula el tiempo restante del timer
        recv_bytes = eth_recv(iface,src_mac,ARP_ETH_TYPE,inbuffer,ETH_MTU,timeleft); //Si el time left llega a '0' se cierra el socket de escucha

        if(recv_bytes == 0){
          printf("eth_send: timeout\n");
          return -1;
        }
        /* 5. Si hemos recibido respuesta extraemos del reply packet la MAC deseada*/
        if(recv_bytes>0){
          reply_packet = NULL;
          reply_packet = (struct arp_pkt_t *) inbuffer;  //casting del paquete a la estruc arp
          memcpy(mac_addr, reply_packet->src_hw_addr,MAC_ADDR_SIZE); //nos guardamos la MAC

        }

        //El siguiente while mira que la ip que nos manda el paquete sea la misma de la que pedimo la MAC y que sea un paquete reply
      }while(!((memcmp(reply_packet->src_proto_addr, ip_addr, IPv4_ADDR_SIZE)==0) & (ntohs(reply_packet->op_code)== ARP_REP_CODE)));

      /*6. Guardamos datos y enseñamos*/
      cache_add(mac_addr,ip_addr);  //Guardamos la entrada en la caché
      cache_show();                 //Mostramos nuesra nueva cache con la entrada añadida
  	return 0; //OK return '0'
  }

/* void cache_init();
 *
 * DESCRIPCIÓN:
 *   Esta función pone a cero el array de la cache
 */
void cache_init(){
  if(!cache_initialized){                                   //Si la cache no esta iniciada
    bzero(cache_table, sizeof(arp_entry_t)*CACHE_LENGTH);   //Rellena de zeros todas las estructuras contenidas en la cache
    cache_initialized = 1;                                  // Marcamos la caché como inicializada
  }
}

/* int cache_resolve(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Resuelve una dirección MAC dando una dirección IP dentro de la caché ARP
 *   Esta funcion hace de arp_resolve pero dentro de la propia caché
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP por la que se pregunta.
 *   'mac_addr': Direccion MAC que se quiere.
 *
 * VALOR DEVUELTO:
 *   Si la encuentra y es actual, devuelve la dirección y 0
 *
 * ERRORES:
 *   Si la encuenta y es antigua, devuelve -1 y borra la direccion
 *   Si no la encuentra, devuelve -2
 */
int cache_resolve(mac_addr_t mac_addr,ipv4_addr_t ip_addr){
  unsigned int index = 0;
  for(index = 0; index<CACHE_LENGTH; index++){                             //Recorre los indices de la cache
    if(memcmp(cache_table[index].ip_addr, ip_addr, IPv4_ADDR_SIZE)==0){   //compara la IP guardada con la deseada
      time_t nowtime = time(NULL);                                        //Guarda en nowtime el TIMESTAMP actual

      //Aqui comparamos el timestamp de cuando la guardamos con el timestamp actual
      if(difftime(nowtime, cache_table[index].last_time) > CACHE_TTL){    //Si ha pasado mas tiempo del TTL
          memcpy(mac_addr, cache_table[index].mac_addr,MAC_ADDR_SIZE);
          bzero(&cache_table[index], sizeof(arp_entry_t));                //Borramos ese espacio de la cache y lo ponemos el struct cero
          return -1;                                                      //Devuleve -1 para proceder a hacer arp_resolve
      }
      else{                                                               //Si el TTL no ha caducado
        memcpy(mac_addr, cache_table[index].mac_addr,MAC_ADDR_SIZE);      //Copia la MAC
        return 0;
      }
    }
  }
  return -2; //Devuelve -2 procede a arp_resolve
}

/* int cache_add(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Esta funcion añade una mac a un espacio vacío del array. 
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP que se quiere guardar.
 *   'mac_addr': Direccion MAC que se quiere guaradar.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si todo ha ido bien.
 *
 */
int cache_add(mac_addr_t mac_addr, ipv4_addr_t ip_addr){
  int err = 0;
  err = cache_add_empty(mac_addr, ip_addr);                               //Intentamos guaradar la entrada en cache
  if(err < 0){                                                            //Si devuleve <0 es que no habia sitio en la cache.
    int older_index = cache_get_older();                                  //Busca el TIME STAMP mas antiguo
    memcpy(cache_table[older_index].ip_addr, ip_addr, IPv4_ADDR_SIZE);    //Copiamos la IP
    memcpy(cache_table[older_index].mac_addr, mac_addr, MAC_ADDR_SIZE);   //Copiamos la MAC
    time_t nowtime = time(NULL);                                          //Cogemos la time STAMP actual
    memcpy(&cache_table[older_index].last_time, &nowtime, sizeof(time_t));//Copiamos la time STAMP actual en la entrada
  }
  return 0;
}

/* int cache_add_empty(mac_addr_t mac_addr,ipv4_addr_t ip_addr);
 *
 * DESCRIPCIÓN:
 *   Si encuentra un slot vacío en la caché (viendo que el timestamp sea 0), 
 *   inserta la entrada arp_cache en ella. Sino devuelve fallo -1
 *
 * PARÁMETROS:
 *   'ip_addr': Direccion IP que se quiere insertar.
 *   'mac_addr': Direccion MAC que se quiere insertar.
 *
 * VALOR DEVUELTO:
 *   Si todo es correct, devuelve 0.
 *
 * ERRORES:
 *   Si ha habido fallos, devuelve -1, por ejemplo falta de estacio
 */
int cache_add_empty(mac_addr_t mac_addr, ipv4_addr_t ip_addr){
  unsigned int index = 0;
  for(index =0; index<CACHE_LENGTH; index++){                                 //Recorre la cache
    if(cache_table[index].last_time==0){                                  // Si la estructura está desocupada
          memcpy(cache_table[index].ip_addr, ip_addr, IPv4_ADDR_SIZE);        //Copiamos la IP
          memcpy(cache_table[index].mac_addr, mac_addr, MAC_ADDR_SIZE);       //Copiamos la MAC
          time_t nowtime = time(NULL);                                        //Cogemos la time actual
          memcpy(&cache_table[index].last_time, &nowtime, sizeof(time_t));    //Copiamos la time actual en la entrada
          return 0;
    }
  }
  return -1;
}

/* int cache_get_older();
 *
 * DESCRIPCIÓN:
 *   Recorre los indices de la caché para ver cual es el mas antiguo.
 *
 * VALOR DEVUELTO:
 *   Devuelve el indice de la entrada mas antigua
 */
int cache_get_older(){
  int older_index = 0;
  double older_lapse = 0;

  unsigned int index = 0;
  for(index =0; index<CACHE_LENGTH; index++){                                 //Recorre la cache
    time_t nowtime = time(NULL);                                              //Set time stamp actual
    double time_difference = difftime(nowtime, cache_table[index].last_time); //Resta LA timestamp de NOW menos la de la entrada de la cahe
    if(time_difference > older_lapse){                                        //Compara las horas y elige la mas antigua
      older_lapse = time_difference;
      older_index = index;
    }
  }
  return older_index; //Devuelve el indice de la entrada mas antigua
}

/* int cache_show();
 *
 * DESCRIPCIÓN:
 *   Muestra los valores que hay en la cache ahora mismo
 *
 * VALOR DEVUELTO:
 *  Si todo es correcto, devuelve 0.
 */
int cache_show(){
  unsigned int index = 0;
  printf("\nCache ARP Actual:\n");
  printf("INDEX\tIP ADDRESS\tMAC ADDRESS\t\tLAST TIME CACHED\n");
  for(index =0; index<CACHE_LENGTH; index++){                                         //Recorre la cache
    printf("%d/%d\t",index,CACHE_LENGTH);
    if(cache_table[index].last_time==0){ //si la entrada tiene un timestamp a 0 significa que está vacia
          printf(" EMPTY\t\tEMPTY\t\t\tEMPTY\n");
    }else{
        char mac_str[MAC_STR_LENGTH];
        mac_addr_str(cache_table[index].mac_addr, mac_str);
        char ip_str[IPv4_STR_MAX_LENGTH];
        ipv4_addr_str(cache_table[index].ip_addr,ip_str);
        printf("%s\t%s\t%f\n",ip_str,mac_str, (double) difftime(time(NULL), cache_table[index].last_time));
    }
  }
  return 0;
}
