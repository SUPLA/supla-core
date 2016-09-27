/*
 ============================================================================
 Name        : gate_module_esp01.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#ifdef __BOARD_gate_module_esp01_ds
	#define B_CFG_PORT          0
	#define B_RELAY1_PORT       3
#else
	#define B_CFG_PORT          2
	#define B_RELAY1_PORT       0
#endif



void ICACHE_FLASH_ATTR supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
	
	ets_snprintf(buffer, buffer_size, "SUPLA-GATE-MODULE-ESP01");
	
}

void ICACHE_FLASH_ATTR supla_esp_board_gpio_init(void) {
		
	supla_input_cfg[0].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;

	// ---------------------------------------
	// ---------------------------------------

    supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
    supla_relay_cfg[0].flags = RELAY_FLAG_RESET;
    supla_relay_cfg[0].channel = 0;
    
}

void ICACHE_FLASH_ATTR supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
	#ifdef __BOARD_gate_module_esp01_ds
		srd->channel_count = 2;
	#else
		srd->channel_count = 1;
	#endif

	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
	srd->channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK;

	srd->channels[0].Default = 0;
	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT);


	#ifdef __BOARD_gate_module_esp01_ds

		srd->channels[1].Number = 1;
	    srd->channels[1].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;

	    srd->channels[1].FuncList = 0;
		srd->channels[1].Default = 0;

		supla_get_temp_and_humidity(srd->channels[1].value);

	#endif

}

void ICACHE_FLASH_ATTR supla_esp_board_send_channel_values_with_delay(void *srpc) {
}
