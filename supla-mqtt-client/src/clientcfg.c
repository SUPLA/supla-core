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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "clientcfg.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/proto.h"
#include "supla-client-lib/tools.h"

char *cfg_id_file = NULL;
char *cfg_authkey_file = NULL;

char *cfg_config_file = NULL;

char cfg_client_GUID[SUPLA_GUID_SIZE];
char cfg_client_AuthKey[SUPLA_AUTHKEY_SIZE];

unsigned char clientcfg_init(int argc, char *argv[]) {
  int a;
  struct passwd *pw;
  char *buffer;
  char GUIDHEX[SUPLA_GUID_HEXSIZE + 1];

  for (a = 0; a < argc; a++) {
    if (strcmp("-config", argv[a]) == 0 && a < argc - 1) {
      cfg_config_file = strdup(argv[a + 1]);
      a++;
    }
  }

  if (cfg_id_file == NULL) {
    pw = getpwuid(getuid());  // NOLINT
    a = strlen(pw->pw_dir) + 50;

    buffer = malloc(a);

    if (snprintf(buffer, a, "%s/.supla-mqtt-client", pw->pw_dir) < 1) {
      free(buffer);
      return 0;
    }

    if (st_file_exists(buffer) == 0) {
      if (mkdir(buffer, 0700) == -1) {
        free(buffer);
        return 0;
      }
    }

    if (snprintf(buffer, a, "%s/.supla-mqtt-client/id", pw->pw_dir) < 1) {
      free(buffer);
      return 0;
    }

    cfg_id_file = strdup(buffer);
    free(buffer);
  }

  if (cfg_id_file == NULL || strlen(cfg_id_file) < 1) {
    supla_log(LOG_ERR, "Unknown id file!");
    return 0;
  }

  if (st_read_guid_from_file(cfg_id_file, cfg_client_GUID, 1) == 0) {
    return 0;
  }

  a = strlen(cfg_id_file) + 10;

  cfg_authkey_file = malloc(a);
  snprintf(cfg_authkey_file, a, "%s.authkey", cfg_id_file);

  if (st_read_authkey_from_file(cfg_authkey_file, cfg_client_AuthKey, 1) == 0) {
    return 0;
  }

  st_guid2hex(GUIDHEX, cfg_client_GUID);
  GUIDHEX[SUPLA_GUID_HEXSIZE] = 0;
  supla_log(LOG_INFO, "Client GUID: %s", GUIDHEX);

  return 1;
}

void clientcfg_free(void) {
  if (cfg_id_file) {
    free(cfg_id_file);
    cfg_id_file = NULL;
  }

  if (cfg_authkey_file) {
    free(cfg_authkey_file);
    cfg_authkey_file = NULL;
  }

  if (cfg_config_file) {
    free(cfg_config_file);
    cfg_config_file = NULL;
  }
}
