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

#include "mqtt_client.h"
#include <unistd.h>
#include "log.h"
#include "sthread.h"

// static
void supla_mqtt_client::job(void *client, void *sthread) {
  static_cast<supla_mqtt_client *>(client)->job(sthread);
}

supla_mqtt_client::supla_mqtt_client(supla_mqtt_client_settings *settings,
                                     supla_mqtt_client_datasource *datasource) {
  this->settings = settings;
  this->datasource = datasource;
  this->sthread = NULL;
}

supla_mqtt_client::~supla_mqtt_client(void) { stop(); }

void supla_mqtt_client::job(void *sthread) {
  while (!sthread_isterminated(sthread)) {
    usleep(1000000);
  }
}

void supla_mqtt_client::start(void) {
  if (sthread == NULL && settings && settings->isMQTTEnabled()) {
    sthread = sthread_simple_run(supla_mqtt_client::job, this, 0);
  }
}

void supla_mqtt_client::stop(void) {
  if (sthread != NULL) {
    sthread_twf(sthread);
    sthread = NULL;
  }
}
