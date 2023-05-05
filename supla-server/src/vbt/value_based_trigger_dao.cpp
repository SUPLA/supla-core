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

#include "value_based_trigger_dao.h"

#include <mysql.h>

#include "log.h"

using std::vector;

supla_value_based_trigger_dao::supla_value_based_trigger_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_value_based_trigger_dao::~supla_value_based_trigger_dao(void) {}

vector<supla_value_based_trigger *> supla_value_based_trigger_dao::get_triggers(
    int user_id) {
  vector<supla_value_based_trigger *> result;

  bool already_connected = dba->is_connected();

  if (!already_connected && !dba->connect()) {
    return result;
  }

  MYSQL_STMT *stmt = NULL;
  const char sql[] =
      "SELECT id, owning_channel_id, IFNULL(channel_id, 0), "
      "IFNULL(channel_group_id, 0), IFNULL(scene_id, 0), IFNULL(schedule_id, "
      "0), IFNULL(push_notification_id, 0), IFNULL(action, 0), action_param, "
      "`trigger` FROM supla_value_based_trigger WHERE user_id = ? AND enabled "
      "= 1";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind[10] = {};

    int id = 0;
    int owning_channel_id = 0;
    int channel_id = 0;
    int channel_group_id = 0;
    int scene_id = 0;
    int schedule_id = 0;
    int push_notification_id = 0;
    int action_id = 0;

    char action_param[256] = {};
    unsigned long action_param_len = 0;
    my_bool action_param_is_null = false;

    char conditions[2049] = {};
    unsigned long conditions_len = 0;
    my_bool conditions_are_null = false;

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;
    rbind[0].buffer_length = sizeof(id);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&owning_channel_id;
    rbind[1].buffer_length = sizeof(channel_id);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&channel_id;
    rbind[2].buffer_length = sizeof(channel_id);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&channel_group_id;
    rbind[3].buffer_length = sizeof(channel_group_id);

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&scene_id;
    rbind[4].buffer_length = sizeof(scene_id);

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&schedule_id;
    rbind[5].buffer_length = sizeof(schedule_id);

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&push_notification_id;
    rbind[6].buffer_length = sizeof(push_notification_id);

    rbind[7].buffer_type = MYSQL_TYPE_LONG;
    rbind[7].buffer = (char *)&action_id;
    rbind[7].buffer_length = sizeof(action_id);

    rbind[8].buffer_type = MYSQL_TYPE_STRING;
    rbind[8].buffer = action_param;
    rbind[8].buffer_length = sizeof(action_param);
    rbind[8].length = &action_param_len;
    rbind[8].is_null = &action_param_is_null;

    rbind[9].buffer_type = MYSQL_TYPE_STRING;
    rbind[9].buffer = conditions;
    rbind[9].buffer_length = sizeof(conditions);
    rbind[9].length = &conditions_len;
    rbind[9].is_null = &conditions_are_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          dba->set_terminating_byte(action_param, sizeof(action_param),
                                    action_param_len, action_param_is_null);

          dba->set_terminating_byte(conditions, sizeof(conditions),
                                    conditions_len, conditions_are_null);

          supla_action_config action_config;

          action_config.set_action_id(action_id);

          if (!action_param_is_null && action_param_len > 0) {
            action_config.apply_json_params(action_param);
          }

          if (channel_id) {
            action_config.set_subject_id(channel_id);
            action_config.set_subject_type(stChannel);
          } else if (channel_group_id) {
            action_config.set_subject_id(channel_group_id);
            action_config.set_subject_type(stChannelGroup);
          } else if (scene_id) {
            action_config.set_subject_id(scene_id);
            action_config.set_subject_type(stScene);
          } else if (schedule_id) {
            action_config.set_subject_id(schedule_id);
            action_config.set_subject_type(stSchedule);
          } else if (push_notification_id) {
            action_config.set_subject_id(push_notification_id);
            action_config.set_subject_type(stPushNotifiction);
          }

          supla_value_based_trigger *vbt = new supla_value_based_trigger(
              id, owning_channel_id, action_config, conditions);

          if (vbt) {
            result.push_back(vbt);
          }
        }
      }
    }

    mysql_stmt_close(stmt);
  }

  if (!already_connected) {
    dba->disconnect();
  }

  return result;
}
