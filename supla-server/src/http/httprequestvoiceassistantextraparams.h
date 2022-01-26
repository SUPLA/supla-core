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

#ifndef HTTP_HTTPREQUESTVOICEASSISTANTEXTRAPARAMS_H_
#define HTTP_HTTPREQUESTVOICEASSISTANTEXTRAPARAMS_H_

#include "http/httprequestextraparams.h"

#define CORRELATIONTOKEN_MAXSIZE 2048
#define GOOGLEREQUESTID_MAXSIZE 512

class supla_http_request_voice_assistant_extra_params
    : public supla_http_request_extra_params {
 private:
  char *correlationToken;
  char *googleRequestId;
  int subChannel;

 public:
  supla_http_request_voice_assistant_extra_params(void);
  supla_http_request_voice_assistant_extra_params(const char correlationToken[],
                                                  const char googleRequestId[]);
  virtual ~supla_http_request_voice_assistant_extra_params(void);
  void setCorrelationToken(const char correlationToken[]);
  const char *getCorrelationTokenPtr(void);
  int getSubChannel(void);
  void setGoogleRequestId(const char googleRequestId[]);
  const char *getGoogleRequestIdPtr(void);
};

#endif /* HTTP_HTTPREQUESTVOICEASSISTANTEXTRAPARAMS_H_ */
