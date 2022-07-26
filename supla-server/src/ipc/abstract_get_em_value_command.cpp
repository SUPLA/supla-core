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

#include "ipc/abstract_get_em_value_command.h"

#include <string.h>

#include "ipc/abstract_ipc_ctrl.h"

using std::string;

supla_abstract_get_em_value_command::supla_abstract_get_em_value_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_get_em_value_command::get_command_name(void) {
  return "GET-EM-VALUE:";
}

void supla_abstract_get_em_value_command::on_command_match(const char *params) {
  process_parameters(
      params, [this](int user_id, int device_id, int channel_id) -> bool {
        supla_channel_electricity_measurement *em =
            get_electricity_measurement(user_id, device_id, channel_id);
        if (!em) {
          return false;
        }

        TElectricityMeter_ExtendedValue_V2 em_ev = {};
        char currency[4];
        em->getMeasurement(&em_ev);
        em->getCurrency(currency);

        unsigned int current1 = em_ev.m[0].current[0];
        unsigned int current2 = em_ev.m[0].current[1];
        unsigned int current3 = em_ev.m[0].current[2];

        if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
            !(em_ev.measured_values & EM_VAR_CURRENT)) {
          current1 *= 10;
          current2 *= 10;
          current3 *= 10;
        }

        _supla_int64_t power_active1 = em_ev.m[0].power_active[0];
        _supla_int64_t power_active2 = em_ev.m[0].power_active[1];
        _supla_int64_t power_active3 = em_ev.m[0].power_active[2];

        if (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW) {
          power_active1 *= 1000;
          power_active2 *= 1000;
          power_active3 *= 1000;
        }

        _supla_int64_t power_reactive1 = em_ev.m[0].power_reactive[0];
        _supla_int64_t power_reactive2 = em_ev.m[0].power_reactive[1];
        _supla_int64_t power_reactive3 = em_ev.m[0].power_reactive[2];

        if (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR) {
          power_reactive1 *= 1000;
          power_reactive2 *= 1000;
          power_reactive3 *= 1000;
        }

        _supla_int64_t power_apparent1 = em_ev.m[0].power_apparent[0];
        _supla_int64_t power_apparent2 = em_ev.m[0].power_apparent[1];
        _supla_int64_t power_apparent3 = em_ev.m[0].power_apparent[2];

        if (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA) {
          power_apparent1 *= 1000;
          power_apparent2 *= 1000;
          power_apparent3 *= 1000;
        }

        char *buffer = NULL;
        size_t size = 0;

        for (char a = 0; a < 2; a++) {
          int n = snprintf(
              buffer, size,
              "VALUE:%i,%i,%i,%i,%i,%u,%u,%u,%lld,%lld,%lld,%lld,%lld,%lld,%"
              "lld,%lld,%lld,%i,%i,%i,%i,%i,%i,%llu,%llu,%llu,%llu,%llu,%llu,%"
              "llu,%llu,%llu,%llu,%llu,%llu,%i,%i,%s",
              em_ev.measured_values, em_ev.m[0].freq, em_ev.m[0].voltage[0],
              em_ev.m[0].voltage[1], em_ev.m[0].voltage[2], current1, current2,
              current3, power_active1, power_active2, power_active3,
              power_reactive1, power_reactive2, power_reactive3,
              power_apparent1, power_apparent2, power_apparent3,
              em_ev.m[0].power_factor[0], em_ev.m[0].power_factor[1],
              em_ev.m[0].power_factor[2], em_ev.m[0].phase_angle[0],
              em_ev.m[0].phase_angle[1], em_ev.m[0].phase_angle[2],
              em_ev.total_forward_active_energy[0],
              em_ev.total_forward_active_energy[1],
              em_ev.total_forward_active_energy[2],
              em_ev.total_reverse_active_energy[0],
              em_ev.total_reverse_active_energy[1],
              em_ev.total_reverse_active_energy[2],
              em_ev.total_forward_reactive_energy[0],
              em_ev.total_forward_reactive_energy[1],
              em_ev.total_forward_reactive_energy[2],
              em_ev.total_reverse_reactive_energy[0],
              em_ev.total_reverse_reactive_energy[1],
              em_ev.total_reverse_reactive_energy[2], em_ev.total_cost,
              em_ev.price_per_unit, currency);

          if (a == 0) {
            if (n > -1) { /* glibc 2.1 */
              size = n + 1;
            } else { /* glibc 2.0 */
              size = IPC_BUFFER_MAX_SIZE;
            }

            if (size) {
              buffer = (char *)malloc(size);
              memset(buffer, 0, size);
            }
          }
        }

        delete em;

        if (buffer) {
          send_result(buffer);
          free(buffer);
          return true;
        }

        return false;
      });
}
