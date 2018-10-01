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

#include "ipcsocket.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include "log.h"
#include "tools.h"

typedef struct {
  int sfd;
  struct sockaddr_un saun, fsaun;
} TSuplaIPC_socket;

void *ipcsocket_init(const char *address) {
  int sfd;
  TSuplaIPC_socket *ipc;

  if (address == 0 || strnlen(address, 110) == 0) {
    supla_log(LOG_ERR, "IPC unknown address");
    return 0;
  }

  if (strnlen(address, 110) > 107) {
    supla_log(LOG_ERR, "IPC address too long");
    return 0;
  }

  if ((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    supla_log(LOG_ERR, "Can't create ipc socket");
    return 0;
  }

  if (st_file_exists(address)) unlink(address);

  ipc = malloc(sizeof(TSuplaIPC_socket));

  if (ipc == NULL) {
    close(sfd);
    return 0;
  }

  memset(ipc, 0, sizeof(TSuplaIPC_socket));
  ipc->sfd = -1;

  ipc->saun.sun_family = AF_UNIX;
  snprintf(ipc->saun.sun_path, sizeof(ipc->saun.sun_path), "%s", address);

  if (bind(sfd, (struct sockaddr *)&ipc->saun, sizeof(ipc->saun)) == -1) {
    free(ipc);
    close(sfd);
    supla_log(LOG_ERR, "Can't bind ipc socket");
    return 0;
  }

  chmod(address, 0770);

  if (listen(sfd, 5) == -1) {
    free(ipc);
    close(sfd);
    supla_log(LOG_ERR, "IPC listen error");
    return 0;
  }

  ipc->sfd = sfd;

  return ipc;
}

int ipcsocket_accept(void *_ipc) {
  assert(_ipc != 0);

  socklen_t fromlen = 0;
  TSuplaIPC_socket *ipc = (TSuplaIPC_socket *)_ipc;
  int client_sd = accept(ipc->sfd, (struct sockaddr *)&ipc->fsaun, &fromlen);

  if (client_sd == -1) {
    supla_log(LOG_ERR, "IPC connection accept error %i", errno);
  } else {
    fcntl(client_sd, F_SETFL, O_NONBLOCK);
  }

  return client_sd;
}

void ipcsocket_close(void *_ipc) {
  TSuplaIPC_socket *ipc = (TSuplaIPC_socket *)_ipc;

  assert(_ipc != 0);

  if (ipc->sfd != -1) {
    close(ipc->sfd);
    ipc->sfd = -1;

    if (st_file_exists(ipc->saun.sun_path)) unlink(ipc->saun.sun_path);
  }
}

void ipcsocket_free(void *_ipc) {
  assert(_ipc != 0);

  ipcsocket_close(_ipc);
  free(_ipc);
}
