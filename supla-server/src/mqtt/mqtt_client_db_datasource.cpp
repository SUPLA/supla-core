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

#include <mqtt_client_db_datasource.h>

#include "log.h"

#ifdef __TEST
#include <assert.h>
#endif /*__TEST*/

supla_mqtt_client_db_datasource::supla_mqtt_client_db_datasource(
    supla_mqtt_client_settings *settings)
    : supla_mqtt_client_datasource(settings) {
  this->mqtt_db = NULL;
  this->pthread = 0;
}

supla_mqtt_client_db_datasource::~supla_mqtt_client_db_datasource(void) {}

void supla_mqtt_client_db_datasource::thread_init(void) {
  supla_mqtt_client_datasource::thread_init();
  database::thread_init();
#ifdef __TEST
  pthread = pthread_self();
#endif /*__TEST*/
}

void supla_mqtt_client_db_datasource::thread_cleanup(void) {
  supla_mqtt_client_datasource::thread_cleanup();
  database::thread_end();
}

bool supla_mqtt_client_db_datasource::db_connect(void) {
#ifdef __TEST
  assert(pthread == pthread_self());
#endif /*__TEST*/

  if (!mqtt_db) {
    mqtt_db = new supla_mqtt_db();

    if (mqtt_db && !mqtt_db->connect()) {
      delete mqtt_db;
      mqtt_db = NULL;
    }
  }

  return mqtt_db != NULL;
}

void supla_mqtt_client_db_datasource::db_disconnect(void) {
  if (mqtt_db) {
    mqtt_db->disconnect();
    delete mqtt_db;
    mqtt_db = NULL;
  }
}

supla_mqtt_db *supla_mqtt_client_db_datasource::get_db(void) { return mqtt_db; }
