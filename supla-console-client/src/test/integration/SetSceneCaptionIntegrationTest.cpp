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

#include "SetSceneCaptionIntegrationTest.h"

#include "log.h"

namespace testing {

using std::string;

SetSceneCaptionIntegrationTest::SetSceneCaptionIntegrationTest()
    : SuperuserAuthorization() {
  expectedResultCode = 0;
  expectedSceneID = 0;
  match = 0;
  memset(expectedCaption, 0, SUPLA_SCENE_CAPTION_MAXSIZE);
}

SetSceneCaptionIntegrationTest::~SetSceneCaptionIntegrationTest() {}

void SetSceneCaptionIntegrationTest::sceneMatch(TSC_SetCaptionResult *result,
                                                TSC_SuplaScene *scene) {
  if (result) {
    ASSERT_EQ(result->ResultCode, expectedResultCode);
    ASSERT_EQ(result->ID, expectedSceneID);

    ASSERT_EQ(
        strncmp(expectedCaption, result->Caption, SUPLA_SCENE_CAPTION_MAXSIZE),
        0);
    match |= 0x1;
  } else if (scene && scene->Id == expectedSceneID) {
    if (strncmp(expectedCaption, scene->Caption, SUPLA_SCENE_CAPTION_MAXSIZE) ==
        0) {
      match |= 0x2;
    }
  }

  if ((match & 0x1) &&
      ((match & 0x2) || expectedResultCode != SUPLA_RESULTCODE_TRUE)) {
    cancelIteration();
  }
}

void SetSceneCaptionIntegrationTest::onSceneCaptionSetResult(
    TSC_SetCaptionResult *result) {
  ASSERT_FALSE(result == NULL);
  sceneMatch(result, NULL);
}

void SetSceneCaptionIntegrationTest::sceneUpdate(TSC_SuplaScene *scene) {
  ASSERT_FALSE(scene == NULL);
  sceneMatch(NULL, scene);
}

void SetSceneCaptionIntegrationTest::reconnect() {
  match = 0;
  SuperuserAuthorization::reconnect();
}

string SetSceneCaptionIntegrationTest::dbGetCaption(int sceneId) {
  char sql[100];
  snprintf(sql, sizeof(sql), "SELECT caption FROM supla_scene WHERE id = %i",
           sceneId);
  return sqlQuery(sql);
}

TEST_F(SetSceneCaptionIntegrationTest, SetCaptionWhtnClientIsNotAuthorized) {
  expectedCaption[0] = 0;
  expectedResultCode = SUPLA_RESULTCODE_UNAUTHORIZED;
  expectedSceneID = 1;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_set_scene_caption(sclient, expectedSceneID, expectedCaption),
      0);

  iterateUntilDefaultTimeout();
}

TEST_F(SetSceneCaptionIntegrationTest, SetFewViarintsOfCaption) {
  expectedResultCode = SUPLA_RESULTCODE_TRUE;
  expectedSceneID = 1;
  ASSERT_FALSE(sclient == NULL);

  superuserAuthorize();

  ASSERT_EQ(dbGetCaption(expectedSceneID).compare("caption\nScene #1\n"), 0);

  snprintf(expectedCaption, SUPLA_SCENE_CAPTION_MAXSIZE, "ABCDF");

  ASSERT_GT(
      supla_client_set_scene_caption(sclient, expectedSceneID, expectedCaption),
      0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedSceneID).compare("caption\nABCDF\n"), 0);

  // Empty -----------------

  reconnect();
  superuserAuthorize();
  expectedCaption[0] = 0;
  ASSERT_GT(
      supla_client_set_scene_caption(sclient, expectedSceneID, expectedCaption),
      0);

  iterateUntilDefaultTimeout();

  ASSERT_EQ(dbGetCaption(expectedSceneID).compare("caption\nNULL\n"), 0);

  // Emoji -----------------
  reconnect();
  superuserAuthorize();
  snprintf(expectedCaption, SUPLA_SCENE_CAPTION_MAXSIZE,
           "Caption with emoji 🤓");
  ASSERT_GT(
      supla_client_set_scene_caption(sclient, expectedSceneID, expectedCaption),
      0);

  iterateUntilDefaultTimeout();

  // FullSize -----------------
  // Database field is limited to 100 characters

  reconnect();
  superuserAuthorize();

  const char LongCaption[] =
      "FullSizeCaption........................................................."
      "........................................................................"
      "........................................................................"
      ".................................123456789";

  snprintf(expectedCaption, SUPLA_SCENE_CAPTION_MAXSIZE, "%s", LongCaption);

  ASSERT_GT(
      supla_client_set_scene_caption(sclient, expectedSceneID, expectedCaption),
      0);
  expectedCaption[255] = 0;

  iterateUntilDefaultTimeout();
  ASSERT_EQ(
      dbGetCaption(expectedSceneID)
          .compare("caption\nFullSizeCaption..................................."
                   "..........................................................."
                   "..........................................................."
                   "..........................................................."
                   "......................123456\n"),
      0);
}
} /* namespace testing */
