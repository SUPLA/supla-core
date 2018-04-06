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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "eh.h"
#include "log.h"
#include "srpc.h"

char debug_mode = 1;
char run_as_daemon = 0;
int fd = -1;
int n = 10;

int supla_read(void *buf, int count, void *sc) {
  count = read(fd, buf, count);
  // supla_log(LOG_DEBUG, "read=%i", count);
  if (count == 0) {
    count = -1;
  }

  return count;
}

int supla_write(void *buf, int count, void *sc) { return count; }

void supla_on_remote_call_received(void *_srpc, unsigned int rr_id,
                                   unsigned int call_type, void *sc,
                                   unsigned char proto_version) {
  supla_log(LOG_DEBUG, "Call: %i, proto: %i, rr_id: %i", call_type,
            proto_version, rr_id);

  TsrpcReceivedData rd;
  srpc_getdata(_srpc, &rd, rr_id);
  n = 10;
}

void supla_on_version_error(void *_srpc, unsigned char remote_version,
                            void *sc) {}

int main(int argc, char **argv) {
  void *srpc = NULL;
  TEventHandler *eh = eh_init();

  if (argc < 2) {
	  supla_log(LOG_ERR, "Use: ./supla-afl input_file");
	  return 1;
  }

  fd = open(argv[1], O_RDONLY);

  if (fd == -1) {
    supla_log(LOG_ERR, "Can't open file /tmp/supla_afl.raw");
    return 1;
  }

  TsrpcParams srpc_params;
  srpc_params_init(&srpc_params);
  srpc_params.user_params = NULL;
  srpc_params.data_read = &supla_read;
  srpc_params.data_write = &supla_write;
  srpc_params.on_remote_call_received = &supla_on_remote_call_received;
  srpc_params.on_version_error = &supla_on_version_error;
  srpc_params.eh = eh;
  srpc = srpc_init(&srpc_params);

  do {
    if (srpc_iterate(srpc) == SUPLA_RESULT_FALSE) {
      // supla_log(LOG_DEBUG, "srpc_iterate(srpc) == SUPLA_RESULT_FALSE");
      break;
    }
    n--;
  } while (srpc_input_dataexists(srpc) == SUPLA_RESULT_TRUE && n > 0);

  srpc_free(srpc);
  eh_free(eh);

  close(fd);

  return 0;
}
