/*
 ============================================================================
 Name        : sthread.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef STHREAD_H_
#define STHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*_func_sthread_initialize)(void *user_data, void *sthread);
typedef void (*_func_sthread_execute)(void *user_data, void *sthread);
typedef void (*_func_sthread_finish)(void *user_data, void *sthread);

typedef struct {

	_func_sthread_initialize initialize;
	_func_sthread_execute execute;
	_func_sthread_finish finish;
	void *user_data;
	char free_on_finish;

}Tsthread_params;

void* sthread_simple_run(_func_sthread_execute execute, void *user_data, char free_on_finish);
void* sthread_run(Tsthread_params *sthread_params);
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
