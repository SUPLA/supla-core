/*
 ============================================================================
 Name        : wifisocket.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifdef __BOARD_wifisocket_54

	#define B_RELAY1_PORT      5
	#define B_CFG_PORT         4

#elif defined(__BOARD_wifisocket_esp01)

	#define B_RELAY1_PORT      0
	#define B_CFG_PORT         2

#elif defined(__BOARD_wifisocket_x4)

	#define B_RELAY1_PORT      12
	#define B_RELAY2_PORT      13
	#define B_RELAY3_PORT      14
	#define B_RELAY4_PORT      16

	#define B_CFG_PORT         0
	#define B_BTN2_PORT        2
	#define B_BTN3_PORT        4
	#define B_BTN4_PORT        5

#else

	#define B_RELAY1_PORT      4
	#define B_CFG_PORT         5

#endif

void ICACHE_FLASH_ATTR supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
	
#ifdef __BOARD_wifisocket_x4
	ets_snprintf(buffer, buffer_size, "SUPLA-SOCKETx4");
#else
	ets_snprintf(buffer, buffer_size, "SUPLA-SOCKET");
#endif
	
}

void ICACHE_FLASH_ATTR supla_esp_board_gpio_init(void) {
		

	#ifdef __BOARD_wifisocket_x4

	supla_input_cfg[0].type = INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;

	supla_input_cfg[1].type = INPUT_TYPE_BUTTON;
	supla_input_cfg[1].gpio_id = B_BTN2_PORT;
	supla_input_cfg[1].flags = INPUT_FLAG_PULLUP;

	supla_input_cfg[2].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[2].gpio_id = B_BTN3_PORT;

	supla_input_cfg[3].type = supla_input_cfg[2].type;
	supla_input_cfg[3].gpio_id = B_BTN4_PORT;

	#else

	supla_input_cfg[0].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;

	#endif

	// ---------------------------------------
	// ---------------------------------------

    supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
    supla_relay_cfg[0].flags = RELAY_FLAG_RESTORE;
    supla_relay_cfg[0].channel = 0;
    
	#ifdef __BOARD_wifisocket_x4

		supla_relay_cfg[1].gpio_id = B_RELAY2_PORT;
		supla_relay_cfg[1].flags = RELAY_FLAG_RESTORE;
		supla_relay_cfg[1].channel = 1;

		supla_relay_cfg[2].gpio_id = B_RELAY3_PORT;
		supla_relay_cfg[2].flags = RELAY_FLAG_RESTORE;
		supla_relay_cfg[2].channel = 2;

		supla_relay_cfg[3].gpio_id = B_RELAY4_PORT;
		supla_relay_cfg[3].flags = RELAY_FLAG_RESTORE;
		supla_relay_cfg[3].channel = 3;

	#endif

}

void ICACHE_FLASH_ATTR  supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
	#if defined(__BOARD_wifisocket_x4)
		srd->channel_count = 4;
	#elif defined(__BOARD_wifisocket_esp01)
		srd->channel_count = 1;
	#else
		srd->channel_count = 2;
	#endif

	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;

	srd->channels[0].FuncList = SUPLA_BIT_RELAYFUNC_POWERSWITCH \
								| SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;

	srd->channels[0].Default = SUPLA_CHANNELFNC_POWERSWITCH;

	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT);

	#ifndef __BOARD_wifisocket_x4
	#ifndef __BOARD_wifisocket_esp01
		srd->channels[1].Number = 1;
		srd->channels[1].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;

		srd->channels[1].FuncList = 0;
		srd->channels[1].Default = 0;

		supla_get_temp_and_humidity(srd->channels[1].value);

	#endif
	#endif

	#ifdef __BOARD_wifisocket_x4

		srd->channels[1].Number = 1;
		srd->channels[1].Type = srd->channels[0].Type;
		srd->channels[1].FuncList = srd->channels[0].FuncList;
		srd->channels[1].Default = srd->channels[0].Default;
		srd->channels[1].value[0] = supla_esp_gpio_relay_on(B_RELAY2_PORT);

		srd->channels[2].Number = 2;
		srd->channels[2].Type = srd->channels[0].Type;
		srd->channels[2].FuncList = srd->channels[0].FuncList;
		srd->channels[2].Default = srd->channels[0].Default;
		srd->channels[2].value[0] = supla_esp_gpio_relay_on(B_RELAY3_PORT);

		srd->channels[3].Number = 3;
		srd->channels[3].Type = srd->channels[0].Type;
		srd->channels[3].FuncList = srd->channels[0].FuncList;
		srd->channels[3].Default = srd->channels[0].Default;
		srd->channels[3].value[0] = supla_esp_gpio_relay_on(B_RELAY4_PORT);

	#endif
}

void ICACHE_FLASH_ATTR supla_esp_board_send_channel_values(void *srpc) {

	supla_esp_channel_value_changed(0, supla_esp_gpio_relay_on(B_RELAY1_PORT));

}
