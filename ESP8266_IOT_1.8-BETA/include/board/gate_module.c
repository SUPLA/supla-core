/*
 ============================================================================
 Name        : gate_module.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#define B_CFG_PORT          5
#define B_RELAY1_PORT       4
#define B_RELAY2_PORT      13

#define B_SENSOR_PORT1     12
#define B_SENSOR_PORT2     14



void ICACHE_FLASH_ATTR supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
	
	ets_snprintf(buffer, buffer_size, "SUPLA-GATE-MODULE");
	
}

void ICACHE_FLASH_ATTR supla_esp_board_gpio_init(void) {
		
	supla_input_cfg[0].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;
	
	supla_input_cfg[1].type = INPUT_TYPE_SENSOR;
	supla_input_cfg[1].gpio_id = B_SENSOR_PORT1;
	supla_input_cfg[1].channel = 2;
	
	supla_input_cfg[2].type = INPUT_TYPE_SENSOR;
	supla_input_cfg[2].gpio_id = B_SENSOR_PORT2;
	supla_input_cfg[2].channel = 3;
	
	// ---------------------------------------
	// ---------------------------------------

    supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
    supla_relay_cfg[0].flags = RELAY_FLAG_RESET;
    supla_relay_cfg[0].channel = 0;
    
    supla_relay_cfg[1].gpio_id = B_RELAY2_PORT;
    supla_relay_cfg[1].flags = RELAY_FLAG_RESET;
    supla_relay_cfg[1].channel = 1;

}

void ICACHE_FLASH_ATTR supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
    srd->channel_count = 5;

	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
	srd->channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATEWAYLOCK \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGATE \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEGARAGEDOOR \
								| SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEDOORLOCK;
	srd->channels[0].Default = 0;
	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT);

	srd->channels[1].Number = 1;
	srd->channels[1].Type = srd->channels[0].Type;
	srd->channels[1].FuncList = srd->channels[0].FuncList;
	srd->channels[1].Default = srd->channels[0].Default;
	srd->channels[1].value[0] = supla_esp_gpio_relay_on(B_RELAY2_PORT);

	srd->channels[2].Number = 2;
	srd->channels[2].Type = SUPLA_CHANNELTYPE_SENSORNO;
	srd->channels[2].FuncList = 0;
	srd->channels[2].Default = 0;
	srd->channels[2].value[0] = 0;

	srd->channels[3].Number = 3;
	srd->channels[3].Type = SUPLA_CHANNELTYPE_SENSORNO;
	srd->channels[3].FuncList = 0;
	srd->channels[3].Default = 0;
	srd->channels[3].value[0] = 0;

	srd->channels[4].Number = 4;

	#if defined(__BOARD_gate_module_dht11)
		srd->channels[4].Type = SUPLA_CHANNELTYPE_DHT11;
	#elif defined(__BOARD_gate_module_dht22)
		srd->channels[4].Type = SUPLA_CHANNELTYPE_DHT22;
	#else
		srd->channels[4].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
	#endif


	srd->channels[4].FuncList = 0;
	srd->channels[4].Default = 0;

	supla_get_temp_and_humidity(srd->channels[4].value);


}

void ICACHE_FLASH_ATTR supla_esp_board_send_channel_values(void *srpc) {

	supla_esp_channel_value_changed(2, gpio__input_get(B_SENSOR_PORT1));
	supla_esp_channel_value_changed(3, gpio__input_get(B_SENSOR_PORT2));

}
