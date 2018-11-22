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

#ifndef AMAZON_ALEXAREQUEST_H_
#define AMAZON_ALEXAREQUEST_H_

#include "amazon/alexaclient.h"
#include "http/httprequest.h"

class supla_alexa_request : public supla_http_request {
 private:
  void *lck;
  supla_alexa_client *client;
  int subChannelFromCorrelationToken;

 public:
  supla_alexa_request(supla_user *user, int ClassID, int DeviceId,
                      int ChannelId, event_source_type EventSourceType);
  virtual ~supla_alexa_request();
  supla_alexa_client *getClient(void);
  int getSubChannelFromCorrelationToken(void);
  virtual bool queueUp(void);
  virtual void terminate(void *sthread);
  virtual bool isEventSourceTypeAccepted(short eventSourceType,
                                         bool verification);
  int getCauseType(void);
  virtual void setCorrelationToken(const char correlationToken[]);
};

#endif /* AMAZON_ALEXAREQUEST_H_ */
