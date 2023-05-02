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
      "SELECT t.id, t.owning_channel_id, c.func, IFNULL(t.channel_id, 0), "
      "IFNULL(t.channel_group_id, 0), IFNULL(t.scene_id, 0), "
      "IFNULL(t.schedule_id, 0), IFNULL(t.push_notification_id, 0), "
      "IFNULL(t.action, 0), t.action_param FROM supla_value_based_trigger t, "
      "supla_dev_channel c WHERE t.user_id = ? AND t.enabled = 1 AND "
      "t.owning_channel_id = c.id AND c.func > 0";

  MYSQL_BIND pbind = {};

  pbind.buffer_type = MYSQL_TYPE_LONG;
  pbind.buffer = (char *)&user_id;

  if (dba->stmt_execute((void **)&stmt, sql, &pbind, 1, true)) {
    MYSQL_BIND rbind[10] = {};

    int id = 0;
    int func = 0;
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

    rbind[0].buffer_type = MYSQL_TYPE_LONG;
    rbind[0].buffer = (char *)&id;
    rbind[0].buffer_length = sizeof(id);

    rbind[1].buffer_type = MYSQL_TYPE_LONG;
    rbind[1].buffer = (char *)&owning_channel_id;
    rbind[1].buffer_length = sizeof(channel_id);

    rbind[2].buffer_type = MYSQL_TYPE_LONG;
    rbind[2].buffer = (char *)&func;
    rbind[2].buffer_length = sizeof(func);

    rbind[3].buffer_type = MYSQL_TYPE_LONG;
    rbind[3].buffer = (char *)&channel_id;
    rbind[3].buffer_length = sizeof(channel_id);

    rbind[4].buffer_type = MYSQL_TYPE_LONG;
    rbind[4].buffer = (char *)&channel_group_id;
    rbind[4].buffer_length = sizeof(channel_group_id);

    rbind[5].buffer_type = MYSQL_TYPE_LONG;
    rbind[5].buffer = (char *)&scene_id;
    rbind[5].buffer_length = sizeof(scene_id);

    rbind[6].buffer_type = MYSQL_TYPE_LONG;
    rbind[6].buffer = (char *)&schedule_id;
    rbind[6].buffer_length = sizeof(schedule_id);

    rbind[7].buffer_type = MYSQL_TYPE_LONG;
    rbind[7].buffer = (char *)&push_notification_id;
    rbind[7].buffer_length = sizeof(push_notification_id);

    rbind[8].buffer_type = MYSQL_TYPE_LONG;
    rbind[8].buffer = (char *)&action_id;
    rbind[8].buffer_length = sizeof(action_id);

    rbind[9].buffer_type = MYSQL_TYPE_STRING;
    rbind[9].buffer = action_param;
    rbind[9].buffer_length = sizeof(action_param);
    rbind[9].length = &action_param_len;
    rbind[9].is_null = &action_param_is_null;

    if (mysql_stmt_bind_result(stmt, rbind)) {
      supla_log(LOG_ERR, "MySQL - stmt bind error - %s",
                mysql_stmt_error(stmt));
    } else {
      mysql_stmt_store_result(stmt);

      if (mysql_stmt_num_rows(stmt) > 0) {
        while (!mysql_stmt_fetch(stmt)) {
          dba->set_terminating_byte(action_param, sizeof(action_param),
                                    action_param_len, action_param_is_null);

          _subjectType_e subject_type = stUnknown;
          int subject_id = 0;
          supla_action_config *action_config = nullptr;

          if (!action_param_is_null && action_param_len > 0) {
            action_config = new supla_action_config();
            action_config->set_action_id(action_id);
            action_config->apply_json_params(action_param);
          }

          if (channel_id) {
            subject_id = channel_id;
            subject_type = stChannel;
          } else if (channel_group_id) {
            subject_id = channel_group_id;
            subject_type = stChannelGroup;
          } else if (scene_id) {
            subject_id = scene_id;
            subject_type = stScene;
          } else if (schedule_id) {
            subject_id = schedule_id;
            subject_type = stSchedule;
          } else if (push_notification_id) {
            subject_id = push_notification_id;
            subject_type = stPushNotifiction;
          }

          supla_value_based_trigger *vbt = new supla_value_based_trigger(
              id, owning_channel_id, func, subject_type, subject_id,
              action_config);

          if (vbt) {
            result.push_back(vbt);
          } else if (action_config) {
            delete action_config;
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
