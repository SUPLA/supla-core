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

#ifndef HTTP_OAUTHCLIENT_H_
#define HTTP_OAUTHCLIENT_H_

#include <http/trivialhttps.h>

class oauth_client : public supla_trivial_https {
 private:
  char *token;
  int expires_at;
  bool token_is_expired;

  char *refresh_token;
  char *oauth_host;
  char *token_endpoint;

 protected:

  virtual bool isExpiredByResult(void) ;
  virtual bool request(const char *method, const char *header,
                       const char *data);

 public:
  oauth_client();
  virtual ~oauth_client();

  void setToken(const char *token, int expires_at);
  void setRefreshToken(const char *refresh_token);
  void setOAuthHost(const char *oauth_host);
  void setOAuthTokenEndpoint(const char *token_endpoint);

  bool isTokenExpired(void);
  bool doRefreshToken();
};

#endif /* HTTP_OAUTHCLIENT_H_ */
