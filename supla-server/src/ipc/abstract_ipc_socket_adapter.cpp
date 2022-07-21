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

#include "ipc/abstract_ipc_socket_adapter.h"

#include <unistd.h>

supla_abstract_ipc_socket_adapter::supla_abstract_ipc_socket_adapter(int sfd) {
  this->sfd = sfd;
  this->eh = NULL;
  if (sfd != -1) {
    eh = eh_init();
    eh_add_fd(eh, sfd);
  }
}

supla_abstract_ipc_socket_adapter::~supla_abstract_ipc_socket_adapter() {
  if (eh) {
    eh_free(eh);
    eh = NULL;
  }
  if (sfd != -1) {
    close(sfd);
    sfd = -1;
  }
}

void supla_abstract_ipc_socket_adapter::wait(int usec) {
  if (eh) {
    eh_wait(eh, usec);
  }
}

int supla_abstract_ipc_socket_adapter::get_sfd(void) { return sfd; }
