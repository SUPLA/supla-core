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

#ifndef AMAZON_ALEXA_CREDENTIALS_H_
#define AMAZON_ALEXA_CREDENTIALS_H_

#include <string>

#include "http/oauth_credentials.h"

class supla_amazon_alexa_credentials : public supla_http_oauth_credentials {
 private:
  std::string region;

 public:
  explicit supla_amazon_alexa_credentials(supla_user *user);
  supla_amazon_alexa_credentials();
  virtual ~supla_amazon_alexa_credentials();

  virtual void remove(void);
  virtual std::string get_region(void);
  void load(void);
  virtual void update(const std::string &access_token,
                      const std::string &refresh_token, int expires_in);
};

#endif /* AMAZON_ALEXACREDENTIALS2_H_ */
