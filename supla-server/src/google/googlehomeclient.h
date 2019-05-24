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

#ifndef GOOGLEHOMECLIENT_H_
#define GOOGLEHOMECLIENT_H_

#include <cstddef>
#include "google/googlehome.h"
#include "voiceassistantclient.h"

class supla_google_home_client : public supla_voice_assistant_client {
 private:
  void *jsonStates;
  bool channelExists(const char *endpointId);
  void *getStateSkeleton(int channelId, short subChannel, bool online);
  void *getHeader(const char requestId[]);
  bool post(void *json_data, int *resultCode);

 public:
  explicit supla_google_home_client(supla_google_home *google_home);
  virtual ~supla_google_home_client(void);
  void clearStateReport(void);
  bool addOnOffState(int channelId, bool on, bool online);
  bool addBrightnessState(int channelId, short brightness, bool online,
                          short subChannel);
  bool addColorState(int channelId, int color, short colorBrightness,
                     bool online, short subChannel);
  bool sendReportState(const char requestId[], int *resultCode = NULL);
  bool requestSync(int *resultCode = NULL);
};

#endif /* GOOGLEHOMECLIENT_H_ */
