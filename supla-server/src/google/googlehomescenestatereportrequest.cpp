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

#include "google/googlehomescenestatereportrequest.h"

#include "log.h"
#include "safearray.h"
#include "svrcfg.h"
#include "user/user.h"

supla_google_home_scene_statereport_request::
    supla_google_home_scene_statereport_request(supla_user *user, int ClassID,
                                                int DeviceId, int ChannelId,
                                                event_type EventType,
                                                const supla_caller &Caller)
    : supla_google_home_request(user, ClassID, DeviceId, ChannelId, EventType,
                                Caller) {
  setDelay(1500000);  // 1.5 sec.
  setTimeout(scfg_int(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT) * 1000);
}

supla_google_home_scene_statereport_request::
    ~supla_google_home_scene_statereport_request(void) {}

bool supla_google_home_scene_statereport_request::queueUp(void) {
  return !duplicateExists && supla_google_home_request::queueUp();
}

bool supla_google_home_scene_statereport_request::verifyExisting(
    supla_http_request *existing) {
  duplicateExists = true;

  set_google_home_request_id(existing);

  existing->setDelay(1000000);
  return true;
}

bool supla_google_home_scene_statereport_request::isCallerAccepted(
    const supla_caller &caller, bool verification) {
  return supla_google_home_request::isCallerAccepted(caller, verification) &&
         caller.get_type() == ctGoogleHome;
}

void supla_google_home_scene_statereport_request::execute(void *sthread) {
  getClient()->clearStateReport();
  getClient()->addSceneState(getChannelId());  // ChannelId == SceneId
  send_report_state();
}

bool supla_google_home_scene_statereport_request::isEventTypeAccepted(
    event_type eventType, bool verification) {
  return eventType == ET_SCENE_EXECUTED || eventType == ET_SCENE_INTERRUPTED;
}

REGISTER_HTTP_REQUEST_CLASS(supla_google_home_scene_statereport_request);
