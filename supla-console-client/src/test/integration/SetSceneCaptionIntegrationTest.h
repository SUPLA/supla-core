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

#ifndef SETSCENECAPTIONINTEGRATIONTEST_H_
#define SETSCENECAPTIONINTEGRATIONTEST_H_

#include <string>

#include "SuperuserAuthorization.h"

namespace testing {

class SetSceneCaptionIntegrationTest : public SuperuserAuthorization {
 private:
  unsigned char match;
  void sceneMatch(TSC_SetCaptionResult *result, TSC_SuplaScene *scene);

 protected:
  unsigned char expectedResultCode;
  int expectedSceneID;
  char expectedCaption[SUPLA_SCENE_CAPTION_MAXSIZE];
  void reconnect();
  std::string dbGetCaption(int sceneId);

 public:
  SetSceneCaptionIntegrationTest();
  virtual ~SetSceneCaptionIntegrationTest();
  virtual void onSceneCaptionSetResult(TSC_SetCaptionResult *result);
  virtual void sceneUpdate(TSC_SuplaScene *scene);
};

} /* namespace testing */

#endif /* SETSCENECAPTIONINTEGRATIONTEST_H_ */
