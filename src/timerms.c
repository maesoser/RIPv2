/* Copyright (C) 2010 Manuel Urue�a <muruenya@it.uc3m.es>
 *
 * This file is part of librawnet.
 *
 * Librawnet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Librawnet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with librawnet.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "timerms.h"

#include <stdlib.h>
#include <sys/time.h>

/* long long int timerms_time();
 *
 * DESCRIPCI�N:
 *   Esta funci�n devuelve el n�mero de milisegundos que han pasado desde el 1
 *   de enero de 1970 hasta el momento actual.
 *
 * VALOR DEVUELTO:
 *   El tiempo actual, medido en milisegundos
 *
 * ERRORES:
 *   La funci�n devuelve -1 si se ha producido un error.
 */
long long int timerms_time()
{
  long long int time_msecs;

  struct timeval timeval;
  int err = gettimeofday(&timeval, NULL);
  if (err != 0) {
    return -1;
  }

  /* Conversion from seconds to milliseconds MUST be done with a
   * long long int */
  long long int time_secs = timeval.tv_sec;
  time_msecs  = time_secs * 1000;
  time_msecs += timeval.tv_usec / 1000;

  return time_msecs;
}


/* long long int timerms_reset ( timerms_t * timer, long int timeout )
 *
 * DESCRIPCI�N:
 *   Esta funci�n inicia un temporizador de 'timeout' milisegundos de
 *   duraci�n. Es posible establecer un temporizador infinito, que no vence
 *   nunca, si se especifica un 'timeout' negativo.
 *
 * PAR�METROS:
 *     'timer': El temporizador que se desea establecer.
 *              La memoria de la estructura 'timerms_t' debe haber sido
 *              reservada previamente.
 *   'timeout': Tiempo en milisegundos que se desea que dure el temporizador
 *              antes de que expire. Un tiempo negativo generar� un
 *              temporizador infinito, que no expirar� nunca.
 *
 * VALOR DEVUELTO:
 *   El tiempo actual, medido en milisegundos.
 *
 * ERRORES:
 *   La funci�n devuelve '-1' si el temporizador es 'NULL' o no se ha
 *   producido un error al obtener el tiempo actual.
 */
long long int timerms_reset ( timerms_t * timer, long int timeout )
{
  long long int now = timerms_time();

  if (timer == NULL) {
    return -1;
  }

  timer->reset_timestamp = now;
  if (timeout >= 0) {
    timer->timeout_timestamp = now + timeout;
  } else { /* timeout < 0 */
    timer->timeout_timestamp = timeout;
  }

  return now;
}

/* long int timerms_elapsed ( timerms_t * timer );
 *
 * DESCRIPCI�N:
 *   Esta funci�n devuelve el tiempo que ha pasado desde la inicializaci�n del
 *   temporizador.
 *
 * PAR�METROS:
 *   'timer': Temporizador que se quiere consultar.
 *            Dicho temporizador debe haber sido inicializado con
 *            'resetTimer()' previamente.
 *
 * VALOR DEVUELTO:
 *   Devuelve el n�mero de milisegundos que han pasado desde que el
 *   temporizador fue inicializado (independientemente de que ya haya expirado
 *   o no).
 *
 * ERRORES:
 *   La funci�n devuelve '-1' si el temporizador es 'NULL' o no se ha
 *   inicializado corectamente.
 */
long int timerms_elapsed ( timerms_t * timer )
{
  long int elapsed_time;

  if (timer == NULL) {
    return -1;
  }

  long long int now = timerms_time();
  elapsed_time = (long int) (now - timer->reset_timestamp);

  return elapsed_time;
}


/* long int timerms_left ( timerms_t * timer )
 *
 * DESCRIPCI�N:
 *   Esta funci�n devuelve el tiempo que le queda al temporizador antes de que
 *   expire. Un tiempo cero indicar� que el temporizador ya ha expirado, y un
 *   tiempo negativo que el temporizador es infinito y no expirar� nunca.
 *
 * PAR�METROS:
 *   'timer': Temporizador que se quiere consultar.
 *            Dicho temporizador debe haber sido inicializado con
 *            'timerms_reset()' previamente.
 *
 * VALOR DEVUELTO:
 *    Devuelve el n�mero de milisegundos que quedan antes de que el
 *    temporizador expire, o 0 si ya ha expirado. Un tiempo negativo (distinto
 *    a -1) indica que el temporizador es infinito y no expirar� nunca.
 *
 * ERRORES:
 *   La funci�n devuelve '-1' si el temporizador es 'NULL' o no se ha
 *   inicializado corectamente.
 */
long int timerms_left ( timerms_t * timer )
{
  long int time_left;

  if (timer == NULL) {
    return -1;
  }

  if (timer->timeout_timestamp < 0) {
    time_left = timer->timeout_timestamp - 1;
  } else {
    long long int now = timerms_time();
    time_left = (long int) (timer->timeout_timestamp - now);
    if (time_left < 0) {
      time_left = 0;
    }
  }

  return time_left;
}
