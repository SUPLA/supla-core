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

#include "GetChannelBasicCfg.h"

namespace testing {

GetChannelBasicCfg::GetChannelBasicCfg() {
  cfgGetDone = false;
  memset(&channelBasicCfg, 0, sizeof(TSC_ChannelBasicCfg));
}

GetChannelBasicCfg::~GetChannelBasicCfg() {}

void GetChannelBasicCfg::getBasicCfg(int ChannelID) {
  cfgGetDone = false;
  memset(&channelBasicCfg, 0, sizeof(TSC_ChannelBasicCfg));
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_get_channel_basic_cfg(sclient, ChannelID), 0);
  iterateUntilDefaultTimeout();
  ASSERT_TRUE(cfgGetDone);
}

void GetChannelBasicCfg::onChannelBasicCfg(TSC_ChannelBasicCfg *cfg) {
  ASSERT_FALSE(cfg == NULL);
  memcpy(&this->channelBasicCfg, cfg, sizeof(TSC_ChannelBasicCfg));
  cfgGetDone = true;
  cancelIteration();
}

} /* namespace testing */
