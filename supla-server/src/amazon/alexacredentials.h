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

#ifndef AMAZON_ALEXACREDENTIALS_H_
#define AMAZON_ALEXACREDENTIALS_H_

#define ALEXA_TOKEN_MAXSIZE 1024
#define ALEXA_REGION_MAXSIZE 5

#include "webhook/webhookbasiccredentials.h"

class supla_user;

class supla_amazon_alexa_credentials : public supla_webhook_basic_credentials {
 private:
  char *region;
  void region_free(void);

 public:
  explicit supla_amazon_alexa_credentials(supla_user *user);
  virtual ~supla_amazon_alexa_credentials();
  virtual int get_token_maxsize(void);
  virtual int get_region_maxsize(void);

  virtual void load();
  virtual void remove();
  virtual void on_credentials_changed();
  void update(const char *access_token, const char *refresh_token,
              int expires_in);
  void set(const char *access_token, const char *refresh_token, int expires_in,
           const char *region);

  char *getRegion(void);
};

#endif /* AMAZON_ALEXACREDENTIALS_H_ */
