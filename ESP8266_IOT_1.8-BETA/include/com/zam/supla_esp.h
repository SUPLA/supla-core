#ifndef SUPLA_ESP_ZAM_H_
#define SUPLA_ESP_ZAM_H_

	#define DEVICE_NAME "ZAM ROW-01"

	#define USE_GPIO16_OUTPUT
	#define LED_RED_PORT     16

	#define RELAY_STATE_RESTORE

	#define RELAY1_PORT      5
	#define CFG_PORT         4

	#define ZAM_INPUT1        14
	#define ZAM_INPUT2        13

	#define BTN1_2_TYPE_SELECTION

	#undef GPIO_PORT_INIT
	#define GPIO_PORT_INIT \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); \
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14)

	#undef GPIO_PORT_POST_INIT
	#define GPIO_PORT_POST_INIT \
		gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);  \
		gpio_pin_intr_state_set(GPIO_ID_PIN(2), GPIO_PIN_INTR_DISABLE);  \
		gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_DISABLE);  \
        gpio_pin_intr_state_set(GPIO_ID_PIN(12), GPIO_PIN_INTR_DISABLE); \
		gpio_pin_intr_state_set(GPIO_ID_PIN(15), GPIO_PIN_INTR_DISABLE); \
		gpio_pin_intr_state_set(GPIO_ID_PIN(16), GPIO_PIN_INTR_DISABLE)


	#undef RELAY_BEFORE_CHANGE_STATE
    #define RELAY_BEFORE_CHANGE_STATE \
		gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_DISABLE);   \
		gpio_pin_intr_state_set(GPIO_ID_PIN(13), GPIO_PIN_INTR_DISABLE);  \
		gpio_pin_intr_state_set(GPIO_ID_PIN(14), GPIO_PIN_INTR_DISABLE)

	#undef RELAY_AFTER_CHANGE_STATE
    #define RELAY_AFTER_CHANGE_STATE  \
        os_delay_us(10000); \
		gpio_pin_intr_state_set(GPIO_ID_PIN(4), GPIO_PIN_INTR_NEGEDGE); \
		supla_esp_gpio_enable_input_port(ZAM_INPUT1); \
		supla_esp_gpio_enable_input_port(ZAM_INPUT2)

#endif /* SUPLA_ESP_ZAM_H_ */
