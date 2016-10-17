/*
 ============================================================================
 Name        : supla_w1.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : 2016 GPLv2
 ============================================================================
*/


#include <os_type.h>
#include <osapi.h>
#include <eagle_soc.h>
#include <gpio.h>

#include "supla_w1.h"

#ifdef W1_GPIO0
  int supla_w1_pin = GPIO_ID_PIN(0);
#elif defined(W1_GPIO5)
  int supla_w1_pin = GPIO_ID_PIN(5);
#elif defined(W1_GPIO3)
  int supla_w1_pin = GPIO_ID_PIN(3);
#elif defined(W1_GPIO14)
  int supla_w1_pin = GPIO_ID_PIN(14);
#else
  int supla_w1_pin = GPIO_ID_PIN(2);
#endif
  
void ICACHE_FLASH_ATTR supla_w1_init(void) {

	#ifdef W1_GPIO0
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
    #elif defined(W1_GPIO5)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
	#elif defined(W1_GPIO3)

	    system_uart_swap ();

		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
	#elif defined(W1_GPIO14)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);
    #else
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
    #endif

    GPIO_DIS_OUTPUT( supla_w1_pin );
}
