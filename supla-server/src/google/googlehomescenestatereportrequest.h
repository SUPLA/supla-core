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

#ifndef GOOGLEHOMESCENESTATEREPORTREQUEST_H_
#define GOOGLEHOMESCENESTATEREPORTREQUEST_H_

#include "google/googlehomerequest.h"

// Google does not officially support status reporting for scenes, but when it
// comes to certification, there are two paths to choose from, one of them (more
// convenient) is to introduce reporting.

class supla_google_home_scene_statereport_request
    : public supla_google_home_request {
 public:
  supla_google_home_scene_statereport_request(supla_user *user, int ClassID,
                                              int DeviceId, int ChannelId,
                                              event_type EventType,
                                              const supla_caller &Caller);
  virtual ~supla_google_home_scene_statereport_request(void);

  virtual bool verifyExisting(supla_http_request *existing);
  virtual bool queueUp(void);
  virtual bool isCallerAccepted(const supla_caller &caller, bool verification);
  virtual bool isEventTypeAccepted(event_type eventType, bool verification);
  virtual void execute(void *sthread);
};

#endif /* GOOGLEHOMESCENESTATEREPORTREQUEST_H_ */
