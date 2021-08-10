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

#ifndef H_TrivialHttpMock_H_
#define H_TrivialHttpMock_H_

#include "http/trivialhttp.h"

class TrivialHttpMock : public supla_trivial_http {
 private:
  static char out_buff[102400];
  char in_buff[102400];

 protected:
  bool send_recv(const char *out, char **in);

 public:
  TrivialHttpMock();
  virtual ~TrivialHttpMock();
  static bool outputEqualTo(const char *str);
};

#endif /*H_TrivialHttpMock_TEST_H_*/
