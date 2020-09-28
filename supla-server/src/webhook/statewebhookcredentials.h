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

#ifndef WEBHOOK_STATEWEBHOOKCREDENTIALS_H_
#define WEBHOOK_STATEWEBHOOKCREDENTIALS_H_

#include <list>
#include "webhook/webhookbasiccredentials.h"

#define WEBHOOK_TOKEN_MAXSIZE 255
#define WEBHOOK_URL_MAXSIZE 255
#define WEBHOOK_FUNCTIONS_IDS_MAXSIZE 255

class supla_state_webhook_credentials : public supla_webhook_basic_credentials {
 private:
  char *url;
  std::list<int> functions_ids;
  void url_free();

 public:
  supla_state_webhook_credentials(supla_user *user);
  ~supla_state_webhook_credentials(void);
  virtual int get_token_maxsize(void);
  void set(const char *access_token, const char *refresh_token, int expires_in,
           const char *url, const char *functions_ids);
  virtual void update(const char *access_token, const char *refresh_token,
                      int expires_in);
  void load();
  void remove(void);
  void on_credentials_changed();
  char *getUrl(void);
  bool isUrlProtocolAccepted(void);
  bool isUrlValid(void);
  char *getHost(void);
  char *getResource(void);
  std::list<int> getFunctionsIds(void);
};

#endif /* WEBHOOK_STATEWEBHOOKCREDENTIALS_H_ */
