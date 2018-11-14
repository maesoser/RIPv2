#include "rip_route_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

//struc de la tabla de rutas
struct ripv2_route_table {
  ripv2_route_t * routes[RIPv2_ROUTE_TABLE_SIZE];
};


/* ripv2_route_t * ripv2_route_create( ipv4_addr_t ip_addr, ipv4_addr_t mask, ipv4_addr_t nh,  uint32_t metric, long long int timeout);
 *
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta RIPv2 con los parámetros especificados:
 *   dirección de subred, máscara, nombre de interfaz y dirección de siguiente
 *   salto.
 *
 *   Esta función reserva memoria para la estructura creada. Debe utilizar la
 *   función 'ripv2_route_free()' para liberar dicha memoria.
 *
 * PARÁMETROS:
 * ip_addr: ip a añadir
 * mask: mascara a añadir
 * nh: next hop a añadir
 * metric: metrica a añadir
 * timeout: timer a añadir
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la ruta creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la ruta.
 */
/*
typedef struct ripv2_route {
    uint16_t addr_id;                                                           //2 bytes si es IP o no
    uint16_t route_tag;                                                         //2 byte
    ipv4_addr_t ip_addr;
    ipv4_addr_t subnet_mask;
    ipv4_addr_t next_hop;
    uint32_t metric;
    timerms_t timer;
} ripv2_route_t;
*/
ripv2_route_t * ripv2_route_create( ipv4_addr_t ip_addr, ipv4_addr_t mask, ipv4_addr_t nh,  uint32_t metric, long long int timeout)
{
  ripv2_route_t * route = (ripv2_route_t *) malloc(sizeof(struct ripv2_route)); //reservamos memoria para una ruta

  if ((route != NULL) && (ip_addr != NULL) && (mask != NULL) ) { //si hemos reservado memoria bien
    route->metric = metric;
    memcpy(route->next_hop, nh, IPv4_ADDR_SIZE);
    memcpy(route->subnet_mask, mask, IPv4_ADDR_SIZE);
    memcpy(route->ip_addr, ip_addr, IPv4_ADDR_SIZE);
    timerms_reset(&route->timer, timeout);

  }

  return route;
}

/* void ripv2_route_print ( ripv2_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime la ruta especificada por la salida estándar.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea imprimir.
 */
void ripv2_route_print ( ripv2_route_t * route )
{
  if (route != NULL) { //si hay ruta

    char sub_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->subnet_mask,sub_str);
    char ip_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->ip_addr,ip_str);
    char nh_str[IPv4_STR_MAX_LENGTH];
    ipv4_addr_str(route->next_hop,nh_str);

    printf("\t%s\t%s\t\t%s\t\t%zu\n",ip_str,sub_str,nh_str,route->metric);
  }
}


/* void ripv2_route_free ( ripv2_route_t * route );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la ruta especificada, que
 *   ha sido creada con 'ripv2_route_create()'.
 *
 * PARÁMETROS:
 *   'route': Ruta que se desea liberar.
 */
void ripv2_route_free ( ripv2_route_t * route )
{
  if (route != NULL) {
    free(route);
  }
}

/* ripv2_route_t* ripv2_route_read ( char* filename, int linenum, char * line )
 *
 * DESCRIPCIÓN:
 *   Esta función crea una ruta RIPv2 a partir de la línea del fichero
 *   de la tabla de rutas especificada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero de la tabla de rutas
 *    'linenum': Número de línea del fichero de la tabal de rutas.
 *       'line': Línea del fichero de la tabla de rutas a procesar.
 *
 * VALOR DEVUELTO:
 *   La ruta leída, o NULL si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función imprime un mensaje de error y devuelve NULL si se ha
 *   producido algún error al leer la ruta.
 */
ripv2_route_t* ripv2_route_read ( char* filename, int linenum, char * line )
{
  ripv2_route_t* route = NULL;

  char ip_addr_str[256];
  char mask_str[256];
  char nh_str[256];
  char metric_str[256];

  /* Parse line: Format "<ip> <mask> <next_hop> <metric>\n" */
  int params = sscanf(line, "%s %s %s %s\n",ip_addr_str, mask_str, nh_str, metric_str);
  if (params != 4) {
    fprintf(stderr, "%s:%d: Invalid RIP Route format: '%s' (%d params)\n",filename, linenum, line, params);
    fprintf(stderr,"%s:%d:Format <ip> <mask> <next_hop> <metric>\n",filename, linenum);
    return NULL;
  }

  uint32_t metric = (uint32_t) atoi(metric_str);

  /* Parse IPv4 route subnet address */
  ipv4_addr_t ip_addr;
  int err = ipv4_str_addr(ip_addr_str, ip_addr);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <addr> value: '%s'\n",
	    filename, linenum, ip_addr_str);
    return NULL;
  }

  /* Parse IPv4 route subnet mask */
  ipv4_addr_t mask;
  err = ipv4_str_addr(mask_str, mask);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <mask> value: '%s'\n",
	    filename, linenum, mask_str);
    return NULL;
  }

  /* Parse IPv4 route gateway */
  ipv4_addr_t nh;
  err = ipv4_str_addr(nh_str, nh);
  if (err == -1) {
    fprintf(stderr, "%s:%d: Invalid <nh> value: '%s'\n",
	    filename, linenum, nh_str);
    return NULL;
  }

  /* Create new route with parsed parameters */
  route = ripv2_route_create( ip_addr, mask, nh,  metric, RIPv2_TIMEOUT);

  if (route == NULL) {
    fprintf(stderr, "%s:%d: Error creating the new route\n",
	    filename, linenum);
  }


  return route;
}


/* void ripv2_route_output ( ripv2_route_t * route, int header, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la ruta RIPv2
 *   especificada.
 *
 * PARÁMETROS:
 *      'route': Ruta a imprimir.
 *     'header': '0' para imprimir una línea con la cabecera de la ruta.
 *        'out': Salida por la que imprimir la ruta.
 *
 * VALOR DEVUELTO:
 *   La función devuelve '0' si la ruta se ha impreso correctamente.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ripv2_route_output ( ripv2_route_t * route, int header, FILE * out )
{
  int err;

  if (header == 0) {
    err = fprintf(out, "IP Addr  \tMask     \t\tNext Hop\tMetric\t Timer\n");
    if (err < 0) {
      return -1;
    }
  }

  char ip_str[IPv4_STR_MAX_LENGTH];
  char mask_str[IPv4_STR_MAX_LENGTH];
  char nh_str[IPv4_STR_MAX_LENGTH];

  if (route != NULL) {
      ipv4_addr_str(route->ip_addr, ip_str);
      ipv4_addr_str(route->subnet_mask, mask_str);
      ipv4_addr_str(route->next_hop, nh_str);

      err = fprintf(out, "%-15s\t%-15s\t\t%-15s\t%d\t\t%lu\n",ip_str, mask_str, nh_str,route->metric,timerms_left(&route->timer));
      if (err < 0) {
        return -1;
      }
  }

  return 0;
}






/* ripv2_route_table_t * ripv2_route_table_create();
 *
 * DESCRIPCIÓN:
 *   Esta función crea una tabla de rutas RipV2 vacía.
 *
 *   Esta función reserva memoria para la tabla de rutas creada, para
 *   liberarla es necesario llamar a la función 'ripv2_route_table_free()'.
 *
 * VALOR DEVUELTO:
 *   La función devuelve un puntero a la tabla de rutas creada.
 *
 * ERRORES:
 *   La función devuelve 'NULL' si no ha sido posible reservar memoria para
 *   crear la tabla de rutas.
 */

ripv2_route_table_t * ripv2_route_table_create(){
  //creamos el puntero
  ripv2_route_table_t * table;
  //reservamos memoria
  table = (ripv2_route_table_t *) malloc(sizeof(struct ripv2_route_table));
  //si se ha creado bien añadimos rutas
  if (table != NULL) {
    int i;
    for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
      table->routes[i] = NULL;
    }
  }

  return table;
}


/* int ripv2_route_table_add ( ripv2_route_table_t * table,
 *                            ripv2_route_t * route );
 * DESCRIPCIÓN:
 *   Esta función añade la ruta especificada en la primera posición libre de
 *   la tabla de rutas.
 *
 * PARÁMETROS:
 *   'table': Tabla donde añadir la ruta especificada.
 *   'route': Ruta a añadir en la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el indice de la posición [0,ripv2_route_TABLE_SIZE-1]
 *   donde se ha añadido la ruta especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no ha sido posible añadir la ruta
 *   especificada.
 */

int ripv2_route_table_add ( ripv2_route_table_t * table, ripv2_route_t * route )
{
  int route_index = -1;

  if (table != NULL) {
    /* Find an empty place in the route table */
    int i;
    for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
      if (table->routes[i] == NULL) {
        table->routes[i] = route;
        route_index = i;
        break;
      }
    }
  }

  return route_index;
}


/* ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table,
 *                                          int index );
 *
 * DESCRIPCIÓN:
 *   Esta función borra la ruta almacenada en la posición de la tabla de rutas
 *   especificada.
 *
 *   Esta función NO libera la memoria reservada para la ruta borrada. Para
 *   ello es necesario utilizar la función 'ripv2_route_free()' con la ruta
 *   devuelta.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea borrar una ruta.
 *   'index': Índice de la ruta a borrar. Debe tener un valor comprendido
 *            entre [0, ripv2_route_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta que estaba almacenada en la posición
 *   indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si la ruta no ha podido ser borrada, o no
 *   existía ninguna ruta en dicha posición.
 */
ripv2_route_t * ripv2_route_table_remove ( ripv2_route_table_t * table, int index )
{
  ripv2_route_t * removed_route = NULL;

  if ((table != NULL) && (index >= 0) && (index < RIPv2_ROUTE_TABLE_SIZE)) {
    removed_route = table->routes[index];
    table->routes[index] = NULL;
  }

  return removed_route;
}


/* ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas especificada.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas de la que se desea obtener una ruta.
 *   'index': Índice de la ruta consultada. Debe tener un valor comprendido
 *            entre [0, ripv2_route_TABLE_SIZE-1].
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la ruta almacenada en la posición de la tabla de
 *   rutas indicada.
 *
 * ERRORES:
 *   Esta función devuelve 'NULL' si no ha sido posible consultar la tabla de
 *   rutas, o no existe ninguna ruta en dicha posición.
 */

ripv2_route_t * ripv2_route_table_get ( ripv2_route_table_t * table, int index )
{
  ripv2_route_t * route = NULL;

  if ((table != NULL) && (index >= 0) && (index < RIPv2_ROUTE_TABLE_SIZE)) {
    route = table->routes[index];
  }

  return route; //devuelve la ruta en esa posicion
}


/* int ripv2_route_table_find ( ripv2_route_table_t * table, ipv4_addr_t subnet,
 *                                                         ipv4_addr_t mask );
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el índice de la ruta para llegar a la subred
 *   especificada.
 *
 * PARÁMETROS:
 *    'table': Tabla de rutas en la que buscar la subred.
 *   'subnet': Dirección de la subred a buscar.
 *     'mask': Máscara de la subred a buscar.
 *
 * VALOR DEVUELTO:
 *   Esta función devuelve la posición de la tabla de rutas donde se encuentra
 *   la ruta que apunta a la subred especificada.
 *
 * ERRORES:
 *   La función devuelve '-1' si no se ha encontrado la ruta especificada o
 *   '-2' si no ha sido posible realizar la búsqueda.
 */

int ripv2_route_table_find( ripv2_route_table_t * table, ipv4_addr_t ip_addr, ipv4_addr_t mask )
{
  int route_index = -2;

  if (table != NULL) {
    route_index = -1;
    int i;
    for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
      ripv2_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        int same_addr = (memcmp(route_i->ip_addr, ip_addr, IPv4_ADDR_SIZE) == 0);
        int same_mask = (memcmp(route_i->subnet_mask, mask, IPv4_ADDR_SIZE) == 0);

        if (same_addr && same_mask) {
          route_index = i; //si coincide la ip y la netmask devuelve el indice
          break;
        }
      }
    }
  }

  return route_index;
}


/* void ripv2_route_table_free ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función libera la memoria reservada para la tabla de rutas
 *   especificada, incluyendo todas las rutas almacenadas en la misma,
 *   mediante la función 'ripv2_route_free()'.
 *
 * PARÁMETROS:
 *   'table': Tabla de rutas a borrar.
 */
void ripv2_route_table_free ( ripv2_route_table_t * table )
{
  if (table != NULL) {
    int i;
    for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
      ripv2_route_t * route_i = table->routes[i];
      if (route_i != NULL) {
        table->routes[i] = NULL;
        ripv2_route_free(route_i);
      }
    }
    free(table);
  }
}


/* int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función lee el fichero especificado y añade las rutas RIP
 *   estáticas leídas en la tabla de rutas indicada.
 *
 * PARÁMETROS:
 *   'filename': Nombre del fichero con rutas IPv4 que se desea leer.
 *      'table': Tabla de rutas donde añadir las rutas leidas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas leidas y añadidas en la tabla, o
 *   '0' si no se ha leido ninguna ruta.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al leer el
 *   fichero de rutas.
 */

int ripv2_route_table_read ( char * filename, ripv2_route_table_t * table )
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
    ripv2_route_t* new_route = ripv2_route_read(filename, linenum, line);
    if (new_route == NULL) {
      err = -1;
      break;
    }

    /* Add new route to Route Table */
    if (table != NULL) {
      err = ripv2_route_table_add(table, new_route);
      if (err >= 0) {
	       err = 0;
	     read_routes++;
      }
    }
  } /* while() */

  if (err == -1) {
    read_routes = -1;
  }

  /* Close rip Route Table file */
  fclose(routes_file);

  return read_routes;
}


/* void ripv2_route_table_output ( ripv2_route_table_t * table, FILE * out );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida indicada la tabla de rutas IPv4
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 *        'out': Salida por la que imprimir la tabla de rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas impresas por la salida indicada, o
 *   '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir por
 *   la salida indicada.
 */
int ripv2_route_table_output ( ripv2_route_table_t * table, FILE * out )
{
  int err;

  int i;
  for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
    ripv2_route_t * route_i = ripv2_route_table_get(table, i);
    if (route_i != NULL) {
      err = ripv2_route_output(route_i, i, out);
      if (err == -1) {
	return -1;
      }
    }
  }

  return 0;
}


/* int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename );
 *
 * DESCRIPCIÓN:
 *   Esta función almacena en el fichero especificado la tabla de rutas IPv4
 *   indicada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a almacenar.
 *   'filename': Nombre del fichero donde se desea almacenar la tabla de
 *               rutas.
 *
 * VALOR DEVUELTO:
 *   La función devuelve el número de rutas almacenadas en el fichero de
 *   rutas, o '0' si la tabla de rutas estaba vacia.
 *
 * ERRORES:
 *   La función devuelve '-1' si se ha producido algún error al escribir el
 *   fichero de rutas.
 */
int ripv2_route_table_write ( ripv2_route_table_t * table, char * filename )
{
  int num_routes = 0;

  FILE * routes_file = fopen(filename, "w");
  if (routes_file == NULL) {
    fprintf(stderr, "Error opening output RIPv2 Routes file \"%s\": %s.\n",filename, strerror(errno));
    return -1;
  }

  fprintf(routes_file, "# %s\n", filename);
  fprintf(routes_file, "#\n");

  if (table != NULL) {
    num_routes = ripv2_route_table_output (table, routes_file);
    if (num_routes == -1) {
      fprintf(stderr, "Error writing RIPv2 Routes file \"%s\": %s.\n",filename, strerror(errno));
      return -1;
    }
  }

  fclose(routes_file);

  return num_routes;
}



/* void ripv2_route_table_print ( ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función imprime por la salida estándar la tabla de rutas RIP
 *   especificada.
 *
 * PARÁMETROS:
 *      'table': Tabla de rutas a imprimir.
 */
void ripv2_route_table_print ( ripv2_route_table_t * table )
{
  if (table != NULL) {
    printf("Tabla RIP actual\n");
    ripv2_route_table_output (table, stdout);
  }
}

/* int ripv2_length(ripv2_route_table_t * table );
 *
 * DESCRIPCIÓN:
 *   Esta función calcula la longitud de la tabla.
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizar.
 * DEVUELVE:
 *      el numero de rutas.
 */
int ripv2_length (ripv2_route_table_t *table ){

  int i = 0;
  for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
    ripv2_route_t * route_i = ripv2_route_table_get(table, i);
    if (route_i == NULL) {
      return i;
    }
  }
  return i;

}

/* timerms_t ripv2_get_min_timer(ripv2_route_table_t * table)
 *
 * DESCRIPCIÓN:
 *   Esta función devuelve el tiempo en ms del timer más cercano a espirar cercano a expirar
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizae.
 * VALOR DEVUELTO:
 *   min time: tiempo minimo encontrado
 */
long int ripv2_get_min_timer(ripv2_route_table_t * table){
  int i = 0;
  long int min_time = RIPv2_TIMEOUT;

  for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
    ripv2_route_t * route_i = ripv2_route_table_get(table, i);
    if (route_i == NULL) {
      return min_time;
    }

    if(timerms_left(&route_i->timer) < min_time){
      min_time = timerms_left(&route_i->timer);
    }

  }
  return min_time;
}

/* int ripv2_clear_table(ripv2_route_table_t * table)
 *
 * DESCRIPCIÓN:
 *   Esta función limpia la tabla de entradas basura o pone a infinito entradas expiradas.
 *
 * PARÁMETROS:
 *      'table': Tablas de rutas a analizar.
 * DEVUELVE:
 *      1 Si alguna entrada está en garbage y debe ser anunciada.
 */
int ripv2_clear_table(ripv2_route_table_t * table){
  int route_changed = 0;
  int i = 0;
  for (i=0; i<RIPv2_ROUTE_TABLE_SIZE; i++) {
    ripv2_route_t * route_i = ripv2_route_table_get(table, i);
    if (route_i != NULL) {
      if(timerms_left(&route_i->timer)==0){ //si el timer se ha acabado
        if(route_i->metric==16){            //si tiene metrica infinita
          ripv2_route_table_remove (table,i);  //borrar ruta
        }
        else{
          route_changed = 1;  //no hay cambios en la ruta sigue caida, no la anuncio inicio garbagge
          ripv2_route_t * updated_route = ripv2_route_table_remove ( table, i );
          updated_route->metric=16;  // mtrica a inf
          timerms_reset(&(updated_route->timer), RIPv2_GARBAGE_TIMEOUT); //pongo el timer del garbagge
          int err = ripv2_route_table_add ( table, updated_route ); //añado la ruta
          if(err < 0){
            printf("ERROR  añadiendo la ruta a la tabla de rip\n");
            exit(-1);
          }
        }
      }
    }
  }
  return route_changed;
}
