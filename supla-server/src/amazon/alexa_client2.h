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

#ifndef AMAZON_ALEXACLIENT_H_
#define AMAZON_ALEXACLIENT_H_

/*
 * Indicates that the event was caused by a customer interaction with an
 * application. For example, a customer switches on a light or locks a door
 * using the Alexa app or an app provided by a device vendor.
 */
#define CAUSE_APP_INTERACTION 0

/*
 * Indicates that the event was caused by a physical interaction with an
 * endpoint. For example, manually switching on a light or manually locking a
 * door lock.
 */
#define CAUSE_PHYSICAL_INTERACTION 1

/*
 * Indicates that the event was caused by the periodic poll of an endpoint,
 * which found a change in value. For example, you might poll a temperature
 * sensor every hour and send the updated temperature to Alexa.
 */
#define CAUSE_PERIODIC_POLL 2

/*
 * Indicates that the event was caused by the application of a device rule. For
 * example, a customer configures a rule to switch on a light if a motion sensor
 * detects motion. In this case, Alexa receives an event from the motion sensor,
 * and another event from the light to indicate that its state change was caused
 * by the rule.
 */
#define CAUSE_RULE_TRIGGER 3

/*
 * Indicates that the event was caused by a voice interaction. For example, a
 * user speaking to their Echo device.
 */
#define CAUSE_VOICE_INTERACTION 4

#include <string>

#include "amazon/alexa_credentials2.h"
#include "http/voice_assistant_client2.h"
#include "json/cJSON.h"

class supla_alexa_client2 : public supla_voice_assistant_client2 {
 private:
  int cause_type;
  cJSON *props;
  void add_props(cJSON *props);
  cJSON *get_percentage_controller_properties(short percentage);
  std::string zulu_time;
  std::string message_id;

 public:
  explicit supla_alexa_client2(int channel_id,
                               supla_abstract_curl_adapter *curl_adapter,
                               supla_amazon_alexa_credentials2 *credentials);
  virtual ~supla_alexa_client2(void);
  void set_cause_type(int cause_type);
  void add_percentage_controller(void);
};

#endif /* AMAZON_ALEXACLIENT_H_ */
