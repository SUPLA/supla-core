/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "client_subscriber.h"
#include <vector>

bool value_exists(jsoncons::json payload, std::string path) {
  try {
    return jsoncons::jsonpointer::contains(payload, path);
  } catch (jsoncons::json_exception& je) {
    return false;
  }
}

void handle_subscribed_message(client_device_channel* channel,
                               std::string topic, std::string message,
                               _func_channelio_valuechanged cb,
                               void* user_data) {
  if (message.length() == 0) return;

  while (!cb) {
    supla_log(LOG_DEBUG, "waiting for registration result...");
    usleep(10);
  }

  supla_log(LOG_DEBUG, "handling message %s", message.c_str());

  char value[SUPLA_CHANNELVALUE_SIZE];

  channel->getValue(value);
  int channelNumber = channel->getNumber();

  jsoncons::json payload;
  std::string template_value = message;

  try {
    payload = jsoncons::json::parse(message);

    if (channel->getPayloadValue().length() > 0) {
      std::string payloadValue = std::string(channel->getPayloadValue());
      template_value =
          jsoncons::jsonpointer::get(payload, payloadValue).as<std::string>();
      if (template_value.length() == 0) return;
    }
  } catch (jsoncons::ser_error& ser) {
  } catch (jsoncons::jsonpointer::jsonpointer_error& error) {
  }
  if (template_value.length() == 0) return;
  
  std::string payloadOn = channel->getPayloadOn();
  std::string payloadOff = channel->getPayloadOff();
  
  if (payloadOn.length() == 0)
	payloadOn = "1";
  if (payloadOff.length() == 0)
	payloadOff = "0";
  
  supla_log(LOG_DEBUG, "handling incomming message: %s",
            template_value.c_str());
  try {
    /* raw payload simple value */
    switch (channel->getFunction()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE: 
	  case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:  // ver. >= 8
      case SUPLA_CHANNELFNC_MAILSENSOR:            // ver. >= 8
	  {
	 	bool hasChanged = false;
		
		if (payloadOn.compare(template_value) == 0)
		{  
			value[0] = 1;
			hasChanged = true;
		}
	    else if (payloadOff.compare(templa_value) == 0)
		{
			value[0] = 0; 
			hasChanged = true;
		};
	 	
		if (hasChanged) {
          channel->setValue(value);
		  channel->setLastSeconds();
		  cchannel->setToggled(false);
    
		  if (cb) cb(channelNumber, value, user_data);
		};
		
		return;
      }
      case SUPLA_CHANNELFNC_DISTANCESENSOR:
      case SUPLA_CHANNELFNC_DEPTHSENSOR:
      case SUPLA_CHANNELFNC_WINDSENSOR:
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
      case SUPLA_CHANNELFNC_RAINSENSOR:
      case SUPLA_CHANNELFNC_WEIGHTSENSOR: {
		double dbval = std::stod(template_value);
        memcpy(value, &dbval, sizeof(double));
        channel->setValue(value);
		
		if (cb) cb(channel->getNumber(), value, user_data);
        return;
      };
      case SUPLA_CHANNELFNC_THERMOMETER: {
        std::string::size_type sz;  // alias of size_t
        double temp = std::stod(template_value, &sz);
		
        channel->setDouble(temp);
        channel->getValue(value);
		 
    
        if (cb) cb(channelNumber, value, user_data);

        return;

      } break;
      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
        std::string::size_type sz;  // alias of size_t
        double temp = std::stod(template_value, &sz);
        double hum = std::stod(template_value.substr(sz));

		channel->setTempHum(temp, hum);
        channel->getValue(value);
		 
    
        if (cb) cb(channelNumber, value, user_data);

        return;

      } break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER: {
        auto temp = atoi(template_value.c_str());
        if (temp < 0) temp = 0;
        if (temp > 100) temp = 100;

        value[0] = temp;

        channel->setValue(value);
		
        if (cb) cb(channel->getNumber(), value, user_data);
        return;
      } break;
    };
	 
  } catch (jsoncons::json_exception& je) {
    supla_log(LOG_ERR, "error while trying get value from payload [error: %s]",
              je.what());
  } catch (std::exception& exception) {
    supla_log(LOG_ERR, "general error %s", exception.what());
  }
}
