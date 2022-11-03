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

#ifndef STHREAD_H_
#define STHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*_func_sthread_initialize)(void *user_data, void *sthread);
typedef void (*_func_sthread_execute)(void *user_data, void *sthread);
typedef void (*_func_sthread_finish)(void *user_data, void *sthread);

typedef struct {
  _func_sthread_initialize initialize;
  _func_sthread_execute execute;
  _func_sthread_finish finish;
  void *user_data;
  char free_on_finish;
} Tsthread_params;

void sthread_simple_run(_func_sthread_execute execute, void *user_data,
                        char free_on_finish, void **sthread);
void sthread_run(Tsthread_params *sthread_params, void **sthread);
void sthread_wait(void *sthread);
char sthread_isterminated(void *sthread);
char sthread_isfinished(void *sthread);
void sthread_terminate(void *sthread);
void sthread_free(void *sthread);
void sthread_twf(void *sthread);

#ifdef __cplusplus
}
#endif

#endif /* STHREAD_H_ */
