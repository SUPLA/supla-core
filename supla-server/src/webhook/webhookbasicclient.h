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

#ifndef WEBHOOK_WEBHOOKBASICCLIENT_H_
#define WEBHOOK_WEBHOOKBASICCLIENT_H_

#include "webhook/webhookbasiccredentials.h"

class supla_trivial_https;

class supla_webhook_basic_client {
 private:
  void httpsInit();
  void *lck;
  supla_trivial_https *https;
  supla_webhook_basic_credentials *credentials;

 protected:
  void httpsFree();
  supla_trivial_https *getHttps(void);
  supla_webhook_basic_credentials *getCredentials(void);
  char *getEndpointId(int channelId, short subChannel);
  void refreshToken(char *host, char *resource);

 public:
  explicit supla_webhook_basic_client(
      supla_webhook_basic_credentials *credentials);
  virtual ~supla_webhook_basic_client();
  void terminate(void);
};

#endif /* WEBHOOK_WEBHOOKBASICCLIENT_H_ */
