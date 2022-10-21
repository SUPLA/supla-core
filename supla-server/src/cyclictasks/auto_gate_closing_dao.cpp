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

#include "cyclictasks/auto_gate_closing_dao.h"

#include <mysql.h>

using std::vector;

supla_auto_gate_closing_dao::supla_auto_gate_closing_dao(
    supla_abstract_db_access_provider *dba) {
  this->dba = dba;
}

supla_auto_gate_closing_dao::~supla_auto_gate_closing_dao() {}

vector<supla_auto_gate_closing_dao::item_t>
supla_auto_gate_closing_dao::get_all_active(void) {
  vector<supla_auto_gate_closing_dao::item_t> result;

  return result;
}

int supla_auto_gate_closing_dao::mark_gate_open(int channel_id) { return 0; }

void supla_auto_gate_closing_dao::mark_gate_closed(int channel_id) {}

void supla_auto_gate_closing_dao::set_closing_attemtp(int channel_id) {}
