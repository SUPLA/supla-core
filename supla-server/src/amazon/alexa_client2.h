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
  typedef struct {
    char *str;
    int code;
  } _alexa_code_t;

  static const _alexa_code_t alexa_codes[];
  static const _alexa_code_t alexa_causes[];

  int cause_type;
  cJSON *props_arr;
  std::string zulu_time;
  std::string message_id;
  std::string correlation_token;
  const char *get_error_string(const int code);
  int get_error_code(const char *code);
  supla_amazon_alexa_credentials2 *get_alexa_credentials(void);
  void add_props(cJSON *props);
  cJSON *add_props(cJSON *props_arr, cJSON *props);
  cJSON *get_power_controller_properties(bool hi);
  cJSON *get_brightness_controller_properties(short brightness);
  cJSON *get_color_controller_properties(int color, short brightness);
  cJSON *get_range_controller_properties(short range);
  cJSON *get_percentage_controller_properties(short percentage);
  cJSON *get_contact_sensor_properties(bool hi);
  cJSON *get_endpoint_health_properties(bool ok);
  cJSON *get_header(const char name[], bool use_correlation_token);
  cJSON *get_endpoint(void);
  cJSON *get_response(void);
  cJSON *get_change_report(void);
  cJSON *get_unrechable_error_response(void);
  int perform_post_request(char *data, int *http_result_code);
  void refresh_token(void);
  int perform_post_request(char *data);

 public:
  explicit supla_alexa_client2(int channel_id,
                               supla_abstract_curl_adapter *curl_adapter,
                               supla_amazon_alexa_credentials2 *credentials,
                               const std::string &zulu_time,
                               const std::string &message_id,
                               const std::string &correlation_token);
  virtual ~supla_alexa_client2(void);
  void set_cause_type(int cause_type);

  void add_power_controller_properties(void);
  void add_brightness_controller_properties(void);
  void add_color_controller_properties(void);
  void add_range_controller(void);
  void add_percentage_controller(void);
  void add_contact_sensor(void);

  bool send_response(void);
  bool send_change_report(void);
};

#endif /* AMAZON_ALEXACLIENT_H_ */
