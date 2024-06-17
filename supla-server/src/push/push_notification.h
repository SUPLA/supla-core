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

#ifndef PUSH_NOTIFICATION_H_
#define PUSH_NOTIFICATION_H_

#include <map>
#include <string>
#include <vector>

#include "json/cJSON.h"
#include "push/pn_recipients.h"

class supla_push_notification {
 private:
  int id;
  std::string title;
  std::string body;
  std::string localized_title;
  std::string localized_body;
  std::string date_time;
  std::vector<std::string> localized_title_args;
  std::vector<std::string> localized_body_args;
  int sound;
  supla_pn_recipients recipients;
  std::map<std::string, std::string> replacement_map;
  std::string apply_replacement_map(std::string str);
  void apply_replacement_map(void);

 public:
  supla_push_notification(void);
  explicit supla_push_notification(int id);
  virtual ~supla_push_notification(void);
  int get_id(void);
  supla_pn_recipients &get_recipients(void);

  virtual void set_title(const std::string &title);
  virtual const std::string &get_title(void);
  virtual void set_body(const std::string &body);
  virtual const std::string &get_body(void);

  virtual void set_localized_title(const std::string &title);
  virtual const std::string &get_localized_title();
  virtual void set_localized_body(const std::string &body);
  virtual const std::string &get_localized_body();

  virtual void set_localized_title_args(const std::vector<std::string> &args);
  virtual const std::vector<std::string> &get_localized_title_args();
  virtual void set_localized_body_args(const std::vector<std::string> &args);
  virtual const std::vector<std::string> &get_localized_body_args();

  virtual void set_sound(int sound);
  virtual int get_sound(void);
  virtual void set_date_time(const std::string date_time);
  std::string get_date_time(void);
  virtual void set_replacement_map(
      std::map<std::string, std::string> *replacement_map);
  virtual bool apply_json(int user_id, cJSON *json);
  virtual bool apply_json(int user_id, const char *json);
};

#endif /* PUSH_NOTIFICATION_H_ */
