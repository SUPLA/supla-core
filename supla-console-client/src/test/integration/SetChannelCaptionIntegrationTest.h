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

#ifndef SETCHANNELCAPTIONINTEGRATIONTEST_H_
#define SETCHANNELCAPTIONINTEGRATIONTEST_H_

#include "GetChannelBasicCfg.h"

namespace testing {

class SetChannelCaptionIntegrationTest : public GetChannelBasicCfg {
 private:
  unsigned char match;
  void channelMatch(TSC_SetChannelCaptionResult *result,
                    TSC_SuplaChannel_C *channel);

 protected:
  unsigned char expectedResultCode;
  int expectedChannelID;
  char expectedCaption[SUPLA_CHANNEL_CAPTION_MAXSIZE];
  void reconnect();

 public:
  SetChannelCaptionIntegrationTest();
  virtual ~SetChannelCaptionIntegrationTest();
  virtual void onChannelCaptionSetResult(TSC_SetChannelCaptionResult *result);
  virtual void channelUpdate(TSC_SuplaChannel_C *channel);
};

} /* namespace testing */

#endif /* SETCHANNELCAPTIONINTEGRATIONTEST_H_ */
