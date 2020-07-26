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

#ifndef VOICEASSISTANTCLIENT_H_
#define VOICEASSISTANTCLIENT_H_

#include "voiceassistant.h"
#include "http/trivialhttp.h"

class supla_voice_assistant_client {
 private:
  void httpClientInit();
  void *lck;
  supla_trivial_http *http;
  supla_voice_assistant *voice_assistant;

 protected:
  void httpClientFree();
  supla_trivial_http *getHttpClient(void);
  supla_voice_assistant *getVoiceAssistant(void);
  char *getEndpointId(int channelId, short subChannel);

 public:
  explicit supla_voice_assistant_client(supla_voice_assistant *voice_assistant);
  virtual ~supla_voice_assistant_client();
  void terminate(void);
};

#endif /* VOICEASSISTANTCLIENT_H_ */
