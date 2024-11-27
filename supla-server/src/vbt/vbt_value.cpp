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

#include "vbt_value.h"

using std::map;
using std::string;

supla_vbt_value::supla_vbt_value(void) {}

supla_vbt_value::~supla_vbt_value(void) {}

bool supla_vbt_value::get_vbt_value(_vbt_var_name_e var_name, double *value) {
  return false;
}

map<string, string> supla_vbt_value::get_replacement_map(void) {
  map<string, string> result;
  return result;
}
