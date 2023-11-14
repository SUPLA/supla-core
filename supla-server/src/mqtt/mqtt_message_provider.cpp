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

#include <mqtt_message_provider.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

supla_mqtt_message_provider::supla_mqtt_message_provider(void) {
  this->index = 0;
}

supla_mqtt_message_provider::~supla_mqtt_message_provider(void) {}

bool supla_mqtt_message_provider::create_message(
    const char *topic_prefix, const char *suid, char **topic_name_out,
    void **message, size_t *message_size, const char *message_string_in,
    bool include_null_byte, const char *topic_name_in, ...) {
  size_t suid_len = 0;
  size_t prefix_len = 0;

  if (topic_name_in == NULL || topic_name_out == NULL || suid == NULL ||
      (suid_len = strnlen(suid, SHORT_UNIQUEID_MAXSIZE)) == 0) {
    return false;
  }

  va_list args;
  memset(&args, 0, sizeof(va_list));

  va_start(args, topic_name_in);
  size_t tn_size = vsnprintf(NULL, 0, topic_name_in, args);
  va_end(args);

  if (tn_size == 0) {
    return false;
  }

  prefix_len =
      topic_prefix ? strnlen(topic_prefix, MQTT_MAX_TOPIC_NAME_SIZE) : 0;
  if (prefix_len > 0) {
    prefix_len++;
  }
  tn_size += prefix_len + suid_len + 8;

  *topic_name_out = (char *)malloc(tn_size);

  if (*topic_name_out) {
    snprintf(*topic_name_out, tn_size, "%s%ssupla/%s/",
             prefix_len > 0 ? topic_prefix : "", prefix_len > 0 ? "/" : "",
             suid);

    size_t offset = prefix_len + suid_len + 7;

    va_start(args, topic_name_in);
    vsnprintf(&(*topic_name_out)[offset], tn_size - offset, topic_name_in,
              args);
    va_end(args);

    if (message && message_string_in) {
      size_t size = strnlen(message_string_in, MQTT_MAX_MESSAGE_SIZE);
      if (include_null_byte) {
        size += 1;
      }
      *message = malloc(size);
      if (*message) {
        memcpy(*message, message_string_in, size);

        if (message_size) {
          *message_size = size;
        }
      }
    }

    return true;
  }

  return false;
}

void supla_mqtt_message_provider::get_mfr_name(int mfr_id, char *buf,
                                               size_t buf_size) {
  switch (mfr_id) {
    case SUPLA_MFR_ACSOFTWARE:
      snprintf(buf, buf_size, "AC SOFTWARE");
      break;
    case SUPLA_MFR_TRANSCOM:
      snprintf(buf, buf_size, "TransCom");
      break;
    case SUPLA_MFR_LOGI:
      snprintf(buf, buf_size, "Logi");
      break;
    case SUPLA_MFR_ZAMEL:
      snprintf(buf, buf_size, "Zamel");
      break;
    case SUPLA_MFR_NICE:
      snprintf(buf, buf_size, "Nice");
      break;
    case SUPLA_MFR_ITEAD:
      snprintf(buf, buf_size, "Itead");
      break;
    case SUPLA_MFR_DOYLETRATT:
      snprintf(buf, buf_size, "Doyle & Tratt");
      break;
    case SUPLA_MFR_HEATPOL:
      snprintf(buf, buf_size, "Heatpol");
      break;
    case SUPLA_MFR_FAKRO:
      snprintf(buf, buf_size, "Fakro");
      break;
    case SUPLA_MFR_PEVEKO:
      snprintf(buf, buf_size, "Peveko");
      break;
    case SUPLA_MFR_WEKTA:
      snprintf(buf, buf_size, "Wekta");
      break;
    case SUPLA_MFR_STA_SYSTEM:
      snprintf(buf, buf_size, "STA-System");
      break;
    case SUPLA_MFR_DGF:
      snprintf(buf, buf_size, "DGF");
      break;
    case SUPLA_MFR_POLIER:
      snprintf(buf, buf_size, "Polier");
      break;
    case SUPLA_MFR_ERGO_ENERGIA:
      snprintf(buf, buf_size, "Ergo Energia");
      break;
    default:
      buf[0] = 0;
      break;
  }
}

void supla_mqtt_message_provider::reset_index(void) { index = 0; }

bool supla_mqtt_message_provider::fetch(const char *topic_prefix,
                                        char **topic_name, void **message,
                                        size_t *message_size) {
  if (get_message_at_index(index, topic_prefix, topic_name, message,
                           message_size) &&
      ((topic_name && *topic_name) || (message && *message))) {
    index++;
    return true;
  }

  return false;
}
