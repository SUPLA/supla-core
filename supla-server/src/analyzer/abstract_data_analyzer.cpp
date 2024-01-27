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

#include "analyzer/abstract_data_analyzer.h"

supla_abstract_data_analyzer::supla_abstract_data_analyzer(void) {
  this->channel_id = 0;
}

supla_abstract_data_analyzer::supla_abstract_data_analyzer(int channel_id) {
  this->channel_id = channel_id;
}

supla_abstract_data_analyzer::~supla_abstract_data_analyzer(void) {}

void supla_abstract_data_analyzer::set_channel_id(int channel_id) {
  this->channel_id = channel_id;
}

int supla_abstract_data_analyzer::get_channel_id(void) { return channel_id; }
