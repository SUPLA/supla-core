/*
 ============================================================================
 Name        : w1.h [1-WIRE]
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef W1_H_
#define W1_H_

#ifdef __cplusplus
extern "C" {
#endif

char w1_ds18b20_get_temp(char *device, double *temp);

#ifdef __cplusplus
}
#endif

#endif /* W1_H_ */
