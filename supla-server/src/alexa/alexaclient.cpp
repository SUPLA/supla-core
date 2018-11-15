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

#include <alexa/alexaclient.h>
#include <http/trivialhttps.h>
#include "log.h"
#include "svrcfg.h"

supla_alexa_client::supla_alexa_client() {}

supla_alexa_client::~supla_alexa_client() {}

#define POST_RESULT_SUCCESS 1

#define POST_RESULT_UNKNOWN_ERROR 0
#define POST_RESULT_INVALID_REQUEST_EXCEPTION  -1
#define POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION  -2
#define POST_RESULT_SKILL_DISABLED_EXCEPTION  -3
#define POST_RESULT_INSUFFICIENT_PERMISSION_EXCEPTION  -4
#define POST_RESULT_SKILL_NOT_FOUND_EXCEPTION  1005
#define POST_RESULT_REQUEST_ENTITY_TOO_LARGE_EXCEPTION  -5
#define POST_RESULT_THROTTLING_EXCEPTION  -6
#define POST_RESULT_INTERNAL_SERVICE_EXCEPTION  -7
#define POST_RESULT_SERVICE_UNAVAILABLE_EXCEPTION  -8
#define POST_RESULT_NOSSL -100

int supla_alexa_client::post(char *data) {

	int result = POST_RESULT_UNKNOWN_ERROR;

#ifdef NOSSL
	return POST_RESULT_NOSSL;
#else
  supla_trivial_https *https = new supla_trivial_https();

  https->setHost(scfg_string(CFG_ALEXA_EVENT_GATEWAY_HOST));
  https->setResource("/v3/events");

  https->http_post();

  if (https->getResultCode() != 200) {

  }

  delete https;
#endif /*NOSSL*/
}

void supla_alexa_client::test(void) {
	post(NULL);
}
