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

#include "test/TrivialHttpMock.h"
#include <string.h>
#include "http/trivialhttp.h"
#include "log.h"

char TrivialHttpMock::out_buff[102400];

// static
bool TrivialHttpMock::outputEqualTo(const char *str) {
  if (strncmp(out_buff, str, sizeof(out_buff)) == 0) {
    return true;
  }

  supla_log(LOG_DEBUG, "OUT: [%s] STR: [%s]", out_buff, str);

  return false;
}

TrivialHttpMock::TrivialHttpMock() {
  snprintf(in_buff, sizeof(in_buff),
           "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
}

bool TrivialHttpMock::send_recv(const char *out, char **in) {
  TrivialHttpMock::out_buff[0] = 0;
  strncpy(out_buff, out, sizeof(out_buff));
  out_buff[sizeof(out_buff) - 1] = 0;
  in_buff[sizeof(in_buff) - 1] = 0;
  *in = strndup(in_buff, sizeof(in_buff));
  return true;
}

TrivialHttpMock::~TrivialHttpMock() {}
