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

#ifndef AMAZON_ALEXACHANGEREPORTREQUEST_H_
#define AMAZON_ALEXACHANGEREPORTREQUEST_H_

#include <amazon/alexarequest.h>

class supla_alexa_changereport_request : public supla_alexa_request {
private:
	bool duplicateExists;
 public:
  supla_alexa_changereport_request(supla_user *user, int ClassID, int DeviceId,
          int ChannelId,
          event_source_type EventSourceType);
  virtual ~supla_alexa_changereport_request();
  virtual bool verifyExisting(supla_http_request *existing);
  virtual bool isEventSourceTypeAccepted(short eventSourceType,
                                         bool verification);
  virtual void execute(void *sthread);
  virtual bool queueUp(void);
};

#endif /* AMAZON_ALEXACHANGEREPORTREQUEST_H_ */
