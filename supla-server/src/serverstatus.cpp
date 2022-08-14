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

#include <connection.h>
#include "serverstatus.h"

#include <stdlib.h>
#include <string.h>
#include "lck.h"
#include "supla-socket.h"

// static
serverstatus *serverstatus::_globalInstance = NULL;

// static
serverstatus *serverstatus::globalInstance(void) {
  if (_globalInstance == NULL) {
    _globalInstance = new serverstatus();
  }

  return _globalInstance;
}

// static
void serverstatus::globalInstanceRelease(void) {
  if (_globalInstance) {
    delete _globalInstance;
    _globalInstance = NULL;
  }
}

serverstatus::serverstatus(void) {
  lck = lck_init();
  last_file = NULL;
  last_line = 0;
}

serverstatus::~serverstatus(void) {
  lck_free(lck);
  if (last_file) {
    free(last_file);
  }
}

void serverstatus::mainLoopHeartbeat(void) {
  lck_lock(lck);
  gettimeofday(&last_mainloop_heartbeat, NULL);
  lck_unlock(lck);
}

void serverstatus::currentLine(const char *file, int line) {
  if (last_file) {
    free(last_file);
    last_file = NULL;
  }

  if (file) {
    last_file = strndup(file, 1024);
  }

  last_line = line;
}

bool serverstatus::getStatus(char *buffer, size_t buffer_size) {
  bool result = false;
  struct timeval now;
  gettimeofday(&now, NULL);

  long int time_sec = ssocket_get_last_accept_error_time_sec();
  if (time_sec) {
    if (now.tv_sec - time_sec < 300) {
      snprintf(buffer, buffer_size, "CONN_ACCEPT_ERR:%i",
               ssocket_get_last_accept_errno());
      return false;
    }
  }

  if (supla_connection::conn_limit_exceeded_hard()) {
    snprintf(buffer, buffer_size, "CONN_LIMIT_EXCEEDED");
    return false;
  }

#ifdef __LCK_DEBUG
  bool main_loop_stuck = false;
#endif /*__LCK_DEBUG*/

  lck_lock(lck);
  if (now.tv_sec - last_mainloop_heartbeat.tv_sec > 30) {
    snprintf(buffer, buffer_size, "MAINLOOP_STUCK:%s:%i",
             last_file ? last_file : "", last_line);
#ifdef __LCK_DEBUG
    main_loop_stuck = true;
#endif /*__LCK_DEBUG*/
  } else {
    snprintf(buffer, buffer_size, "OK");
    result = true;
  }
  lck_unlock(lck);

#ifdef __LCK_DEBUG
  if (main_loop_stuck) {
    lck_debug_dump();
  }
#endif /*__LCK_DEBUG*/
  return result;
}
