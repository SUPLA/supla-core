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

#ifndef FCM_ACCESS_TOKEN_PROVIDER_H_
#define FCM_ACCESS_TOKEN_PROVIDER_H_

#include <openssl/evp.h>

#include <string>

#include "json/cJSON.h"
#include "push/abstract_access_token_provider.h"

class supla_fcm_access_token_provider
    : public supla_abstract_access_token_provider {
 private:
  std::string client_email;
  std::string token_uri;
  std::string base64_url_encoded(char *str, int len);
  std::string base64_url_encoded(cJSON *json);
  std::string jwt_sign(void);
  EVP_PKEY *private_key;
  BIO *pk_bio;
  void initialize(void);

 protected:
  virtual bool new_token(supla_abstract_curl_adapter **_curl_adapter,
                         std::string *token, int *expires_in_secs);

 public:
  explicit supla_fcm_access_token_provider(const char *key_file_path);
  virtual ~supla_fcm_access_token_provider(void);

  virtual std::string get_url(void);
};

#endif /* FCM_ACCESS_TOKEN_PROVIDER_H_ */
