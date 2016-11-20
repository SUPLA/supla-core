/*
 ============================================================================
 Name        : sonoff_th.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#define B_RELAY1_PORT    12
#define B_CFG_PORT        0

void supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
	#ifdef __BOARD_sonoff_th10
		ets_snprintf(buffer, buffer_size, "SONOFF-TH10");
	#else
		ets_snprintf(buffer, buffer_size, "SONOFF-TH16");
	#endif
}


void supla_esp_board_gpio_init(void) {
		
	supla_input_cfg[0].type = INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;
	supla_input_cfg[0].relay_gpio_id = B_RELAY1_PORT;
	supla_input_cfg[0].channel = 0;

	// ---------------------------------------
	// ---------------------------------------

    supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
    supla_relay_cfg[0].flags = RELAY_FLAG_RESTORE;
    supla_relay_cfg[0].channel = 0;
    

}

void supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
    srd->channel_count = 3;

	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;

	srd->channels[0].FuncList = SUPLA_BIT_RELAYFUNC_POWERSWITCH \
								| SUPLA_BIT_RELAYFUNC_LIGHTSWITCH;

	srd->channels[0].Default = SUPLA_CHANNELFNC_POWERSWITCH;

	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT);


	srd->channels[1].Number = 1;
	srd->channels[1].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;

	srd->channels[1].FuncList = 0;
	srd->channels[1].Default = 0;

	supla_get_temperature(srd->channels[1].value);

	srd->channels[2].Number = 2;
	srd->channels[2].Type = SUPLA_CHANNELTYPE_AM2301;

	srd->channels[2].FuncList = 0;
	srd->channels[2].Default = 0;

	supla_get_temp_and_humidity(srd->channels[2].value);


}

void supla_esp_board_send_channel_values_with_delay(void *srpc) {

	supla_esp_channel_value_changed(0, supla_esp_gpio_relay_on(B_RELAY1_PORT));

}
