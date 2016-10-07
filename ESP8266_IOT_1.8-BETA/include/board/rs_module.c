/*
 ============================================================================
 Name        : rs_module.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Copyright   : GPLv2
 ============================================================================
*/

#define B_CFG_PORT          0
#define B_RELAY1_PORT       4
#define B_RELAY2_PORT       5
#define B_BTN1_PORT        13
#define B_BTN2_PORT        14
#define B_SENSOR_PORT1     12


void supla_esp_board_set_device_name(char *buffer, uint8 buffer_size) {
	
	ets_snprintf(buffer, buffer_size, "SUPLA-RS-MODULE");
	
}

void supla_esp_board_gpio_init(void) {
		
	supla_input_cfg[0].type = INPUT_TYPE_BUTTON;
	supla_input_cfg[0].gpio_id = B_CFG_PORT;
	supla_input_cfg[0].flags = INPUT_FLAG_PULLUP | INPUT_FLAG_CFG_BTN;
	
	supla_input_cfg[1].type = INPUT_TYPE_SENSOR;
	supla_input_cfg[1].gpio_id = B_SENSOR_PORT1;
	supla_input_cfg[1].channel = 1;
	
	supla_input_cfg[2].type = INPUT_TYPE_BUTTON_HILO;
	supla_input_cfg[2].gpio_id = B_BTN1_PORT;
	supla_input_cfg[2].relay_gpio_id = B_RELAY1_PORT;

	supla_input_cfg[3].type = INPUT_TYPE_BUTTON_HILO;
	supla_input_cfg[3].gpio_id = B_BTN2_PORT;
	supla_input_cfg[3].relay_gpio_id = B_RELAY2_PORT;

	// ---------------------------------------
	// ---------------------------------------

    supla_relay_cfg[0].gpio_id = B_RELAY1_PORT;
    supla_relay_cfg[0].flags = RELAY_FLAG_RESET | RELAY_FLAG_TURNOFF_BINDED;
    supla_relay_cfg[0].channel = 0;
    supla_relay_cfg[0].bind = 1;
    
    supla_relay_cfg[1].gpio_id = B_RELAY2_PORT;
    supla_relay_cfg[1].flags = RELAY_FLAG_RESET | RELAY_FLAG_TURNOFF_BINDED;
    supla_relay_cfg[1].bind = 0;

}

void supla_esp_board_set_channels(TDS_SuplaRegisterDevice_B *srd) {
	
    srd->channel_count = 3;

	srd->channels[0].Number = 0;
	srd->channels[0].Type = SUPLA_CHANNELTYPE_RELAY;
	srd->channels[0].FuncList =  SUPLA_BIT_RELAYFUNC_CONTROLLINGTHEROLLERSHUTTER;
	srd->channels[0].Default = SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER;
	srd->channels[0].value[0] = supla_esp_gpio_relay_on(B_RELAY1_PORT) ? 1 : ( supla_esp_gpio_relay_on(B_RELAY2_PORT) ? 2 : 0 ) ;

	srd->channels[1].Number = 1;
	srd->channels[1].Type = SUPLA_CHANNELTYPE_SENSORNO;


	srd->channels[2].Number = 2;
	srd->channels[2].Type = SUPLA_CHANNELTYPE_THERMOMETERDS18B20;
	srd->channels[2].FuncList = 0;
	srd->channels[2].Default = 0;

	supla_get_temp_and_humidity(srd->channels[2].value);


}

void supla_esp_board_send_channel_values_with_delay(void *srpc) {
	supla_esp_channel_value_changed(1, gpio__input_get(B_SENSOR_PORT1));
}
