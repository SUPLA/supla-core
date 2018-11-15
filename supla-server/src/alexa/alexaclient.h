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

#ifndef ALEXA_ALEXACLIENT_H_
#define ALEXA_ALEXACLIENT_H_

class supla_alexa_token;

class supla_alexa_client {
 private:
  supla_alexa_token *alexa_token;
  void refresh_roken();

 protected:
  int parseErrorCode(const char *code);
  int aeg_post_request(char *data);
  int aeg_post(char *data);

 public:
  supla_alexa_client(supla_alexa_token *alexa_token);
  virtual ~supla_alexa_client();

  void test(void);
};

#endif /* ALEXA_ALEXACLIENT_H_ */
