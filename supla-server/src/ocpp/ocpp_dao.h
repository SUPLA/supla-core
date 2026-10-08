// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SUPLA_OCPP_DAO_H_
#define SUPLA_OCPP_DAO_H_

#include <vector>

#include "ocpp/ocpp_device.h"

class supla_mariadb_access_provider;

class supla_ocpp_dao {
 private:
  supla_mariadb_access_provider *dba;

 public:
  explicit supla_ocpp_dao(supla_mariadb_access_provider *dba);
  // A failed query is distinct from successful removal of a station.
  bool get_devices(int user_id, int device_id,
                   std::vector<supla_ocpp_device_config> *devices);
  bool save_value(int user_id, const supla_ocpp_channel &channel);
  bool save_extended_value(int user_id, const supla_ocpp_channel &channel);
  bool renew_validity(int user_id, int device_id, unsigned int seconds);
};

#endif  // SUPLA_OCPP_DAO_H_
