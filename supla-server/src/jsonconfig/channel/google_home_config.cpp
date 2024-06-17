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

#include "google_home_config.h"

// static
const char google_home_config::root_key[] = "googleHome";

// static
const char google_home_config::value_key[] = "googleHomeDisabled";

google_home_config::google_home_config(void)
    : supla_abstract_voice_assistant_config() {}

google_home_config::google_home_config(supla_json_config *root)
    : supla_abstract_voice_assistant_config(root) {}

const char *google_home_config::get_root_key(void) { return root_key; }

const char *google_home_config::get_value_key(void) { return value_key; }
