/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "sthread.h"
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "lck.h"
#include "log.h"

typedef struct {
  pthread_t _thread;
  char terminated;
  char finished;
  void *lck;
  Tsthread_params params;
} Tsthread;

void *_sthread_run(void *ptr) {
  Tsthread *sthread = (Tsthread *)ptr;

  if (sthread->params.initialize != 0)
    sthread->params.user_data =
        sthread->params.initialize(sthread->params.user_data, ptr);

  if (sthread->params.execute != 0)
    sthread->params.execute(sthread->params.user_data, ptr);

  if (sthread->params.finish != 0)
    sthread->params.finish(sthread->params.user_data, ptr);

  lck_lock(sthread->lck);
  sthread->finished = 1;
  lck_unlock(sthread->lck);

  if (sthread->params.free_on_finish) sthread_free(sthread);

  return 0;
}

void *sthread_run(Tsthread_params *sthread_params) {
  Tsthread *sthread = malloc(sizeof(Tsthread));
  int r = -1;

  if (sthread == NULL) return NULL;

  memset(sthread, 0, sizeof(Tsthread));

  sthread->lck = lck_init();
  memcpy(&sthread->params, sthread_params, sizeof(Tsthread_params));

  if ((r = pthread_create(&sthread->_thread, NULL, &_sthread_run, sthread)) !=
      0) {
    supla_log(LOG_ERR, "Could not create a new thread. Error code: %i", r);
  }

  return sthread;
}


void *sthread_simple_run(_func_sthread_execute execute, void *user_data,
                         char free_on_finish) {
  Tsthread_params p;
  p.user_data = user_data;
  p.free_on_finish = free_on_finish;
  p.execute = execute;
  p.finish = NULL;
  p.initialize = NULL;

  return sthread_run(&p);
}

void sthread_wait(void *sthread) {
  pthread_join(((Tsthread *)sthread)->_thread, NULL);
}

char sthread_isterminated(void *sthread) {
  char result = 0;

  lck_lock(((Tsthread *)sthread)->lck);
  result = ((Tsthread *)sthread)->terminated;
  lck_unlock(((Tsthread *)sthread)->lck);

  return result;
}

char sthread_isfinished(void *sthread) {
  char result = 0;

  lck_lock(((Tsthread *)sthread)->lck);
  result = ((Tsthread *)sthread)->finished;
  lck_unlock(((Tsthread *)sthread)->lck);

  return result;
}

void sthread_terminate(void *sthread) {
  lck_lock(((Tsthread *)sthread)->lck);
  ((Tsthread *)sthread)->terminated = 1;
  lck_unlock(((Tsthread *)sthread)->lck);

  pthread_kill(((Tsthread *)sthread)->_thread, SIGINT);
}

void sthread_free(void *sthread) {
  pthread_detach(((Tsthread *)sthread)->_thread);
  lck_free(((Tsthread *)sthread)->lck);
  free((Tsthread *)sthread);
}

void sthread_twf(void *sthread) {
  sthread_terminate(sthread);
  sthread_wait(sthread);
  sthread_free(sthread);
}
