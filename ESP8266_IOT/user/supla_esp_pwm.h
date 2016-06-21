/*
 ============================================================================
 Name        : supla_esp_pwm.h
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include "supla_esp.h"

#ifdef SUPLA_PWM_COUNT

#ifndef SUPLA_ESP_PWM_H_
#define SUPLA_ESP_PWM_H_

#include <eagle_soc.h>
#include <os_type.h>


void supla_esp_pwm_init(void);
void supla_esp_pwm_set_percent_duty(uint8 percent, uint8 channel);

#endif /* SUPLA_PWM_COUNT */

#endif /* SUPLA_PWM_H_ */