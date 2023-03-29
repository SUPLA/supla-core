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

#include "alexa_config.h"

// static
const char alexa_config::root_key[] = "alexa";

// static
const char alexa_config::value_key[] = "alexaDisabled";

alexa_config::alexa_config(void) : supla_abstract_voice_assistant_config() {}

alexa_config::alexa_config(channel_json_config *root)
    : supla_abstract_voice_assistant_config(root) {}

const char *alexa_config::get_root_key(void) { return root_key; }

const char *alexa_config::get_value_key(void) { return value_key; }
