#include <stdio.h>
#include <timerms.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>	// Time htons etc etc
#include "eth.h"
#include "ipv4.h"
#include "arp.h"

/* Logitud máxmima del nombre de un interfaz de red */
#define IFACE_NAME_MAX_LENGTH 32
#define ARP_TABLE_SIZE 256 /* Número de entradas máximo de la tabla de rutas IPv4 */

ipv4_addr_t ip_addr;
eth_iface_t *iface;

typedef struct arp_table {
  arp_entry_t * entries[ARP_TABLE_SIZE];
} arp_table_t;

typedef struct arp_entry{
	mac_addr_t mac_addr;
	ipv4_addr_t ip_addr;
}arp_entry_t;


arp_entry_t * arp_create( ipv4_addr_t ip_addr, mac_addr_t mac_addr){
  arp_entry_t * entry = (arp_entry_t *) malloc(sizeof(struct arp_entry));
  if ((ip_addr != NULL) && (mac_addr != NULL) ) {
    memcpy(entry->ip_addr, ip_addr, IPv4_ADDR_SIZE);
    memcpy(entry->mac_addr, mac_addr, MAC_ADDR_SIZE);
  }
  return entry;
}

void arp_print ( arp_entry_t * entry ){
  if (entry != NULL) {
    char ip_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(entry->ip_addr,ip_str);
    char mac_str[MAC_STR_MAX_LENGTH];
    mac_addr_str(entry->mac_addr,mac_str);
    printf("%s\t%s\n",ip_str,mac_str);
  }
}

void arp_free ( arp_entry_t * entry )
{
  if (entry != NULL) free(entry);
}

arp_entry_t* arp_read ( char* filename, int linenum, char * line )
{
  arp_entry_t* entry = NULL;

  char ip_addr_str[256];
  char mac_addr_str[256];

  /* Parse line: Format "<ip> <mask> <next_hop> <metric>\n" */
  int params = sscanf(line, "%s %s\n",ip_addr_str, mac_addr_str);
  if (params != 2) {
    fprintf(stderr, "%s:%d: Invalid ARP Entry format: '%s' (%d params)\n",filename, linenum, line, params);
    fprintf(stderr,"%s:%d:Format <ip> <mac>\n",filename, linenum);
    return NULL;
  }

  /* Parse IPv4 route subnet address */
  ipv4_addr_t ip_addr;
  int err = ipv4_str_addr(ip_addr_str, ip_addr);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <addr> value: '%s'\n",filename, linenum, ip_addr_str);
    return NULL;
  }

  /* Parse MAC addr */
  mac_addr_t mac_addr;
  err = mac_str_addr(mac_addr_str, mac_addr);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <mac> value: '%s'\n",
	    filename, linenum, mask_str);
    return NULL;
  }

  /* Create new route with parsed parameters */
  entry = arp_create( ip_addr, mac_addr);

  if (route == NULL) {
    fprintf(stderr, "%s:%d: Error creating the new route\n",filename, linenum);
  }
  return route;
}

arp_table_t * arp_table_create(){
  arp_table_t * table = (arp_table_t *) malloc(sizeof(struct arp_table));
  if (table != NULL) {
    int i;
    for (i=0; i<arp_TABLE_SIZE; i++) {
      table->entries[i] = NULL;
    }
  }
  return table;
}

int arp_table_add ( arp_table_t * table, arp_entry_t * entry )
{
  int route_index = -1;

  if (table != NULL) {
    /* Find an empty place in the route table */
    int i;
    for (i=0; i<ARP_TABLE_SIZE; i++) {
      if (table->entries[i] == NULL) {
        table->entries[i] = entry;
        route_index = i;
        break;
      }
    }
  }

  return route_index;
}

arp_entry_t * arp_table_remove ( arp_table_t * table, int index )
{
  arp_entry_t * removed_route = NULL;
  if ((table != NULL) && (index >= 0) && (index < ARP_TABLE_SIZE)) {
    removed_entry = table->entries[index];
    table->entries[index] = NULL;
  }
  return removed_route;
}

arp_entry_t * arp_table_get ( arp_table_t * table, int index )
{
  arp_entry_t * route = NULL;
  if ((table != NULL) && (index >= 0) && (index < ARP_TABLE_SIZE)) {
    route = table->routes[index];
  }
  return route;
}

int arp_table_find( arp_table_t * table, ipv4_addr_t ip_addr)
{
  int route_index = -2;

  if (table != NULL) {
    route_index = -1;
    int i;
    for (i=0; i<ARP_TABLE_SIZE; i++) {
      arp_entry_t * route_i = table->entries[i];
      if (route_i != NULL) {
        int same_addr = (memcmp(route_i->ip_addr, ip_addr, IPv4_ADDR_SIZE) == 0);
        if (same_addr && same_mask) {
          route_index = i;
          break;
        }
      }
    }
  }

  return route_index;
}

void arp_table_free ( arp_table_t * table )
{
  if (table != NULL) {
    int i;
    for (i=0; i<ARP_TABLE_SIZE; i++) {
      arp_entry_t * route_i = table->entries[i];
      if (route_i != NULL) {
        table->entries[i] = NULL;
        arp_free(route_i);
      }
    }
    free(table);
  }
}

int arp_table_read ( char * filename, arp_table_t * table )
{
  int read_routes = 0;

  FILE * routes_file = fopen(filename, "r");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening input IPv4 Routes file \"%s\": %s.\n",filename, strerror(errno));
    return -1;
  }

  int linenum = 0;
  char line_buf[1024];
  int err = 0;

  while ((! feof(routes_file)) && (err==0)) {

    linenum++;

    /* Read next line of file */
    char* line = fgets(line_buf, 1024, routes_file);
    if (line == NULL) {
      break;
    }

    /* If this line is empty or a comment, just ignore it */
    if ((line_buf[0] == '\n') || (line_buf[0] == '#')) {
      err = 0;
      continue;
    }

    /* Parse route from line */
    arp_entry_t* new_route = arp_read(filename, linenum, line);
    if (new_route == NULL) {
      err = -1;
      break;
    }

    /* Add new route to Route Table */
    if (table != NULL) {
      err = arp_table_add(table, new_route);
      if (err >= 0) {
	       err = 0;
	     read_routes++;
      }
    }
  } /* while() */

  if (err == -1) {
    read_routes = -1;
  }

  /* Close IP Route Table file */
  fclose(routes_file);

  return read_routes;
}

char mac_string[MAC_STR_LENGTH];
char ip_string[IPv4_STR_MAX_LENGTH];

int main(int argc,char *argv[]){

  arp_table_t arp_table = arp_table_create();

	// Si no hay suficientes argumentos
	if(argc != 4){
		printf("Escriba %s [IF] [IP] [RIP_FILE]\n", argv[0]);
		return -1;
	}
	// Si la IP está mal
  iface = eth_open(argv[1]);
	if(ipv4_str_addr ( argv[2], ip_addr )){
		printf("El argumento %s no es una IP válida\n",argv[1]);
		return -1;
	}
  int len = arp_table_read(argv[3],arp_table);
  if(len=0){
    printf("No hay rutas en este fichero");
    return -1;
  }

  while(1){
    
  }
	eth_close(iface);//CErramos eth.
	return 0;
}
