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

#include "test/STTrivialHttp.h"
#include "http/trivialhttp.h"
#include "log.h"

#define OUTDATA_MAXSIZE 102400
#define INDATA_MAXSIZE 102400

char th_test_out_buff[OUTDATA_MAXSIZE];
char th_test_in_buff[INDATA_MAXSIZE];

bool th_test_send_recv(const char *out, char **in, bool *result) {
  th_test_out_buff[0] = 0;
  strncpy(th_test_out_buff, out, OUTDATA_MAXSIZE);
  th_test_out_buff[OUTDATA_MAXSIZE - 1] = 0;
  th_test_in_buff[INDATA_MAXSIZE - 1] = 0;
  *in = strndup(th_test_in_buff, INDATA_MAXSIZE);
  *result = true;
  return true;
}

bool STTrivialHttp::outputEqualTo(const char *str) {
  if (strncmp(th_test_out_buff, str, OUTDATA_MAXSIZE) == 0) {
    return true;
  }

  supla_log(LOG_DEBUG, "OUT: [%s] STR: [%s]", th_test_out_buff, str);

  return false;
}

STTrivialHttp::STTrivialHttp() {
  snprintf(th_test_in_buff, INDATA_MAXSIZE,
           "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");

  supla_trivial_http::extern_send_recv = &th_test_send_recv;
}

STTrivialHttp::~STTrivialHttp() {}
