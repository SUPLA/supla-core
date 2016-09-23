/*
 ============================================================================
 Name        : dht_esp01.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/


void ICACHE_FLASH_ATTR supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
    #ifdef __BOARD_dht11_esp01
	ets_snprintf(buffer, buffer_size, "SUPLA-DHT11-ESP01");
	#elif defined(__BOARD_dht22_esp01)
	ets_snprintf(buffer, buffer_size, "SUPLA-DHT22-ESP01");
	#elif defined(__BOARD_am2302_esp01)
	ets_snprintf(buffer, buffer_size, "SUPLA-AM2302-ESP01");
	#endif
}

	
void supla_esp_board_gpio_init(void) {
		
	supla_input_cfg[0].type = INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = 0;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;

}

void supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	

	srd->channel_count = 1;
	srd->channels[0].Number = 0;

	#ifdef __BOARD_dht11_esp01
	  srd->channels[0].Type = SUPLA_CHANNELTYPE_DHT11;
	#elif defined(__BOARD_dht22_esp01)
	  srd->channels[0].Type = SUPLA_CHANNELTYPE_DHT22;
	#elif defined(__BOARD_am2302_esp01)
	  srd->channels[0].Type = SUPLA_CHANNELTYPE_AM2302;
	#endif

	srd->channels[0].FuncList = 0;
	srd->channels[0].Default = 0;

	supla_get_temp_and_humidity(srd->channels[0].value);



}

void supla_esp_board_send_channel_values(void *srpc) {
}
