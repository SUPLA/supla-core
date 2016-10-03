/*
 ============================================================================
 Name        : lightswitch.c
 Author      : Zybi
 Copyright   : GPLv2
 ============================================================================
*/

void supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {

 #if defined(__BOARD_lightswitch_x2) || defined(__BOARD_lightswitch_x2_54)

	ets_snprintf(buffer, buffer_size, "SUPLA-LIGHTSWITCH-x2-DS");

  #elif defined(__BOARD_lightswitch_x2_DHT11) || defined(__BOARD_lightswitch_x2_54_DHT11)

	ets_snprintf(buffer, buffer_size, "SUPLA-LIGHTSWITCH-x2-DHT11");

  #elif defined(__BOARD_lightswitch_x2_DHT22) || defined(__BOARD_lightswitch_x2_54_DHT22)

	ets_snprintf(buffer, buffer_size, "SUPLA-LIGHTSWITCH-x2-DHT22");

 #endif

}


void supla_esp_board_gpio_init(void) {

	supla_input_cfg[0].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;
	supla_input_cfg[0].relay_gpio_id = B_RELAY1_PORT;
	supla_input_cfg[0].channel = 0;
	
	supla_input_cfg[1].type = supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? INPUT_TYPE_SWITCH : INPUT_TYPE_BUTTON;
	supla_input_cfg[1].gpio_id = B_BTN2_PORT;
	supla_input_cfg[1].flags = INPUT_FLAG_PULLUP;
	supla_input_cfg[1].relay_gpio_id = B_RELAY2_PORT;
	supla_input_cfg[1].channel = 1;

// -----------------------------------------------------------
	supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
	supla_relay_cfg[0].flags = RELAY_FLAG_RESET;
//	supla_relay_cfg[0].flags = RELAY_FLAG_RESTORE;
	supla_relay_cfg[0].channel = 0;
	
	supla_relay_cfg[1].gpio_id = B_RELAY2_PORT;
	supla_relay_cfg[1].flags = RELAY_FLAG_RESET;
//	supla_relay_cfg[1].flags = RELAY_FLAG_RESTORE;
	supla_relay_cfg[1].channel = 1;

}


void supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
	srd->channel_count = 3;
	
	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
	srd->channels[0].FuncList = SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;
	srd->channels[0].Default = SUPLA_CHANNELFNC_LIGHTSWITCH;
	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT);

	srd->channels[1].Number = 1;
	srd->channels[1].Type = srd->channels[0].Type;
	srd->channels[1].FuncList = srd->channels[0].FuncList;
	srd->channels[1].Default = srd->channels[0].Default;
	srd->channels[1].value[0] = supla_esp_gpio_relay_on(B_RELAY2_PORT);

	srd->channels[2].Number = 2;

	#if defined(DS18B20)

		srd->channels[2].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;

	#elif defined(SENSOR_DHT11)

		srd->channels[2].Type = SUPLA_CHANNELTYPE_DHT11;

	#elif defined(SENSOR_DHT22)

		srd->channels[2].Type = SUPLA_CHANNELTYPE_DHT22;

	#endif

	srd->channels[2].FuncList = 0;
	srd->channels[2].Default = 0;

	supla_get_temp_and_humidity(srd->channels[2].value);

}


void supla_esp_board_send_channel_values_with_delay(void *srpc) {

	supla_esp_channel_value_changed(0, supla_esp_gpio_relay_on(B_RELAY1_PORT));
	supla_esp_channel_value_changed(1, supla_esp_gpio_relay_on(B_RELAY2_PORT));

}

