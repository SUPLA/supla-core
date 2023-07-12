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

#ifndef SETCHANNELGROUPCAPTIONINTEGRATIONTEST_H_
#define SETCHANNELGROUPCAPTIONINTEGRATIONTEST_H_

#include <string>

#include "GetChannelBasicCfg.h"

namespace testing {

class SetChannelGroupCaptionIntegrationTest : public GetChannelBasicCfg {
 private:
  unsigned char match;
  void channelGroupMatch(TSC_SetCaptionResult *result,
                         TSC_SuplaChannelGroup_B *channel_group);

 protected:
  unsigned char expectedResultCode;
  int expectedChannelGroupID;
  char expectedCaption[SUPLA_CHANNEL_GROUP_CAPTION_MAXSIZE];
  void reconnect();
  std::string dbGetCaption(int groupId);

 public:
  SetChannelGroupCaptionIntegrationTest();
  virtual ~SetChannelGroupCaptionIntegrationTest();
  virtual void onChannelCaptionSetResult(TSC_SetCaptionResult *result);
  virtual void channelGroupUpdate(TSC_SuplaChannelGroup_B *channel_group);
};

} /* namespace testing */

#endif /* SETCHANNELCAPTIONINTEGRATIONTEST_H_ */
