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

#ifndef SUPLA_ITRIVIALHTTP_H_
#define SUPLA_ITRIVIALHTTP_H_

class supla_itrivial_http_client {
 public:
  supla_itrivial_http_client();
  virtual void setHost(char *host, bool copy = true) = 0;
  virtual void setPort(int port) = 0;
  virtual void setResource(char *resource, bool copy = true) = 0;
  virtual int getResultCode(void) = 0;
  virtual int getContentLength(void) = 0;
  virtual const char *getContentType(void) = 0;
  virtual const char *getBody(void) = 0;
  virtual void setToken(char *token, bool copy = true) = 0;
  virtual bool http_get(void) = 0;
  virtual bool http_post(char *header, char *data) = 0;
  virtual void terminate(void) = 0;
};

#endif /* SUPLA_ITRIVIALHTTP_H_ */
