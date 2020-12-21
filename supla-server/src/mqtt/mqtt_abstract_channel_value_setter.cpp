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

#include "mqtt_abstract_channel_value_setter.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

supla_mqtt_abstract_channel_value_setter::
    supla_mqtt_abstract_channel_value_setter(
        supla_mqtt_client_settings *settings) {
  this->settings = settings;
  this->topic_name = NULL;
  this->topic_name_size = 0L;
  this->message = NULL;
  this->message_size = 0;
  this->channel_id = 0;
  this->email_ptr = NULL;
  this->email_len = 0;
  this->email = NULL;
}

supla_mqtt_abstract_channel_value_setter::
    ~supla_mqtt_abstract_channel_value_setter(void) {
  email_free();
}

bool supla_mqtt_abstract_channel_value_setter::lc_equal(const char *str) {
  for (size_t a = 0; a < message_size; a++) {
    if (tolower(str[a]) != tolower(message[a])) {
      return false;
    }
  }

  return true;
}

bool supla_mqtt_abstract_channel_value_setter::parse_on(void) {
  if (topic_name_size == 6 &&
      memcmp(topic_name, "set/on", topic_name_size) == 0) {
    if ((message_size == 1 && message[0] == '1') ||
        (message_size == 3 && lc_equal("yes")) ||
        (message_size == 4 && lc_equal("true"))) {
      set_on(true);
    } else if ((message_size == 1 && message[0] == '0') ||
               (message_size == 2 && lc_equal("no")) ||
               (message_size == 5 && lc_equal("false"))) {
      set_on(false);
    }

    return true;
  }

  return false;
}

bool supla_mqtt_abstract_channel_value_setter::parse_perecntage(void) {
  bool closing_percentage = false;

  if (topic_name_size == 22 &&
      memcmp(topic_name, "set/closing_percentage", topic_name_size) == 0) {
    closing_percentage = true;
  } else if (topic_name_size != 22 ||
             memcmp(topic_name, "set/opening_percentage", topic_name_size) !=
                 0) {
    return false;
  }

  bool err = false;
  int _percent = parse_int(message, message_size, &err);
  if (!err && _percent >= 0 && _percent <= 100) {
    if (!closing_percentage) {
      _percent = 100 - _percent;
    }
    char percent = _percent;
    action_shut(&percent);
  }

  return true;
}

bool supla_mqtt_abstract_channel_value_setter::parse_action(void) {
  if (topic_name_size == 14 &&
      memcmp(topic_name, "execute_action", topic_name_size) == 0) {
    if (message_size == 7 && lc_equal("turn_on")) {
      set_on(true);
    } else if (message_size == 8 && lc_equal("turn_off")) {
      set_on(false);
    } else if (message_size == 6 && lc_equal("toggle")) {
      action_toggle();
    } else if (message_size == 4 && lc_equal("shut")) {
      action_shut(NULL);
    } else if (message_size == 6 && lc_equal("reveal")) {
      action_reveal();
    } else if (message_size == 4 && lc_equal("stop")) {
      action_stop();
    } else if (message_size == 10 && lc_equal("open_close")) {
      action_open_close();
    }
    return true;
  }
  return false;
}

bool supla_mqtt_abstract_channel_value_setter::parse_brightness(void) {
  bool color = false;

  if (topic_name_size == 20 &&
      memcmp(topic_name, "set/color_brightness", topic_name_size) == 0) {
    color = true;
  } else if (topic_name_size != 14 ||
             memcmp(topic_name, "set/brightness", topic_name_size) != 0) {
    return false;
  }

  bool err = false;
  int percent = parse_int(message, message_size, &err);
  if (!err && percent >= 0 && percent <= 100) {
    if (color) {
      set_color_brightness(percent);
    } else {
      set_brightness(percent);
    }
  }

  return true;
}

int supla_mqtt_abstract_channel_value_setter::parse_int(const char *str,
                                                        size_t len, bool *err) {
  int result = 0;
  bool minus = false;
  bool dot = false;
  size_t a = 0;
  size_t _len = len;

  for (a = 0; a < len; a++) {
    if (a == 0 && str[a] == '-') {
      minus = true;
      continue;
    } else if (a > 0 && !dot && str[a] == '.') {
      dot = true;
      _len = a;
    } else if (str[a] < '0' || str[a] > '9') {
      if (err) {
        *err = true;
      }
      return 0;
    }
  }

  for (a = minus ? 1 : 0; a < _len; a++) {
    result += (str[a] - '0') * pow(10, _len - 1 - a);
  }

  if (minus) {
    result *= -1;
  }

  return result;
}

void supla_mqtt_abstract_channel_value_setter::email_free(void) {
  if (email) {
    free(email);
    email = NULL;
  }
}

const char *supla_mqtt_abstract_channel_value_setter::get_email(void) {
  email_free();
  if (email_len && email_ptr) {
    email = (char *)malloc(email_len + 1);
    memcpy(email, email_ptr, email_len);
    email[email_len] = 0;
  }

  return email;
}

int supla_mqtt_abstract_channel_value_setter::get_channel_id(void) {
  return channel_id;
}

void supla_mqtt_abstract_channel_value_setter::set_value(char *topic_name,
                                                         size_t topic_name_size,
                                                         char *message,
                                                         size_t message_size) {
  if (topic_name == NULL || topic_name_size == 0 || message == NULL ||
      message_size == 0) {
    return;
  }

  size_t prefix_len =
      settings->getPrefix() ? strnlen(settings->getPrefix(), 512) : 0;

  if (prefix_len) {
    if (topic_name_size <= prefix_len) {
      return;
    }

    if (memcmp(topic_name, settings->getPrefix(), prefix_len) != 0) {
      return;
    }

    topic_name += prefix_len;
    topic_name_size -= prefix_len;

    if (memcmp(topic_name, "/", 1) != 0) {
      return;
    }

    topic_name++;
    topic_name_size--;
  }

  if (memcmp(topic_name, "supla/", 6) != 0) {
    return;
  }

  topic_name += 6;
  topic_name_size -= 6;

  email_len = 0;
  email_ptr = NULL;

  bool at = false;
  size_t a = 0;

  for (a = 0; a < topic_name_size; a++) {
    if (!at && topic_name[a] == '@') {
      at = true;
    } else if (topic_name[a] == '/') {
      if (at) {
        email_len = a;
        email_ptr = (char *)topic_name;
        break;
      } else {
        return;
      }
    }
  }

  topic_name += email_len + 1;
  topic_name_size -= email_len + 1;

  if (memcmp(topic_name, "channels/", 9) != 0) {
    return;
  }

  topic_name += 9;
  topic_name_size -= 9;
  channel_id = 0;

  for (a = 0; a < topic_name_size; a++) {
    if (topic_name[a] == '/') {
      if (a == 0) {
        return;
      }

      bool err = false;
      channel_id = parse_int(topic_name, a, &err);

      if (err) {
        return;
      }

      topic_name += a + 1;
      topic_name_size -= a + 1;
      break;
    }
  }

  if (channel_id == 0) {
    return;
  }

  this->topic_name = topic_name;
  this->topic_name_size = topic_name_size;
  this->message = message;
  this->message_size = message_size;

  if (parse_on()) {
    return;
  }

  if (parse_perecntage()) {
    return;
  }

  if (parse_action()) {
    return;
  }

  if (parse_brightness()) {
    return;
  }
}
