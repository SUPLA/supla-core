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

#include "ipc/abstract_calcfg_ipc_command.h"

supla_abstract_calcfg_ipc_command::supla_abstract_calcfg_ipc_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

void supla_abstract_calcfg_ipc_command::send_calcfg_result(
    const char *ok_result, int ok_id, int queued_id,
    unsigned _supla_int64_t since, bool waiting_for_result) {
  if (since) {
    send_result(waiting_for_result ? "WAITING_FOR_RESULT:" : "QUEUED:",
                queued_id, since);
  } else {
    send_result(ok_result, ok_id);
  }
}
