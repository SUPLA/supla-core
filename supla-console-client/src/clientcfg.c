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

char *cfg_email = NULL;
char *cfg_host = NULL;
int cfg_port = 0;
char cfg_ssl_enabled = 1;

int cfg_aid = 0;
char *cfg_pwd = NULL;

char cfg_client_GUID[SUPLA_GUID_SIZE];
char cfg_client_AuthKey[SUPLA_AUTHKEY_SIZE];

int lifetime = 0;
char input_off = 0;

unsigned char proto_version = 0;

unsigned char clientcfg_init(int argc, char *argv[]) {
  int a, x;
  struct passwd *pw;
  char *buffer;
  char GUIDHEX[SUPLA_GUID_HEXSIZE + 1];

  for (a = 0; a < argc; a++) {
    if (strcmp("-i", argv[a]) == 0 && a < argc - 1) {
      cfg_id_file = strdup(argv[a + 1]);
      a++;
    } else if (strcmp("-h", argv[a]) == 0 && a < argc - 1) {
      cfg_host = strdup(argv[a + 1]);
      a++;
    } else if (strcmp("-tcp", argv[a]) == 0) {
      cfg_ssl_enabled = 0;
    } else if (strcmp("-p", argv[a]) == 0 && a < argc - 1) {
      cfg_port = atoi(argv[a + 1]);
      a++;
    } else if (strcmp("-aid", argv[a]) == 0 && a < argc - 1) {
      cfg_aid = atoi(argv[a + 1]);
      a++;
    } else if (strcmp("-pwd", argv[a]) == 0 && a < argc - 1) {
      cfg_pwd = strdup(argv[a + 1]);
      a++;
    } else if (strcmp("-email", argv[a]) == 0 && a < argc - 1) {
      cfg_email = strdup(argv[a + 1]);
      a++;
    } else if (strcmp("-proto", argv[a]) == 0 && a < argc - 1) {
      x = atoi(argv[a + 1]);
      if (x >= SUPLA_PROTO_VERSION_MIN && x <= SUPLA_PROTO_VERSION) {
        proto_version = x;
      }
      a++;
    } else if (strcmp("-lifetime", argv[a]) == 0 && a < argc - 1) {
      lifetime = atoi(argv[a + 1]);
      a++;
    } else if (strcmp("-input", argv[a]) == 0 && a < argc - 1) {
      input_off = strcmp("off", argv[a + 1]) == 0;
      a++;
    }
  }

  if (cfg_port == 0) cfg_port = cfg_ssl_enabled == 1 ? 2016 : 2015;

  if (cfg_host == NULL) cfg_host = strdup("127.0.0.1");

  if (cfg_id_file == NULL) {
    pw = getpwuid(getuid());  // NOLINT
    a = strlen(pw->pw_dir) + 50;

    buffer = malloc(a);

    if (snprintf(buffer, a, "%s/.supla-client", pw->pw_dir) < 1) {
      free(buffer);
      return 0;
    }

    if (st_file_exists(buffer) == 0) {
      if (mkdir(buffer, 0700) == -1) {
        free(buffer);
        return 0;
      }
    }

    if (snprintf(buffer, a, "%s/.supla-client/id", pw->pw_dir) < 1) {
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

  if (cfg_host) {
    free(cfg_host);
    cfg_host = NULL;
  }

  if (cfg_pwd) {
    free(cfg_pwd);
    cfg_pwd = NULL;
  }

  if (cfg_email) {
    free(cfg_email);
    cfg_email = NULL;
  }
}
