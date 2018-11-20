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

#ifndef AMAZON_ALEXARESPONSEREQUEST_H_
#define AMAZON_ALEXARESPONSEREQUEST_H_

#include <http/httprequest.h>

class supla_alexa_response_request : public supla_http_request {
 public:
  supla_alexa_response_request(supla_user *user, int ClassID);
  virtual ~supla_alexa_response_request();
  virtual bool verifyExisting(supla_http_request *existing);
  virtual bool queueUp(void);
  virtual bool isEventSourceTypeAccepted(short eventSourceType,
                                         bool verification);
};

#endif /* AMAZON_ALEXARESPONSEREQUEST_H_ */
