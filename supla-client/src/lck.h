/*
 ============================================================================
 Name        : lck.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef LCK_H_
#define LCK_H_

#ifdef __cplusplus
extern "C" {
#endif

void *lck_init(void);
void lck_lock(void *lck);
char lck_lock_with_timeout(void *lck, int timeout_sec);
void lck_unlock(void *lck);
int lck_unlock_r(void *lck, int result);
void lck_free(void *lck);

#ifdef __cplusplus
}
#endif

#endif /* LCK_H_ */
