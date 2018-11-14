/*
 * oauthclient.h
 *
 *  Created on: Nov 14, 2018
 *      Author: user01
 */

#ifndef HTTP_OAUTHCLIENT_H_
#define HTTP_OAUTHCLIENT_H_

#include <http/trivialhttps.h>

class oauth_client : public supla_trivial_https {
 public:
  oauth_client();
  virtual ~oauth_client();
};

#endif /* HTTP_OAUTHCLIENT_H_ */
