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

#include "ActionShadingSystemParametersTest.h"

#include "actions/action_shading_system_parameters.h"

namespace testing {

ActionShadingSystemParametersTest::ActionShadingSystemParametersTest(void) {}

ActionShadingSystemParametersTest::~ActionShadingSystemParametersTest(void) {}

TEST_F(ActionShadingSystemParametersTest, contructor) {
  supla_action_shading_system_parameters sp1;
  EXPECT_EQ(sp1.get_percentage(), -1);
  EXPECT_EQ(sp1.get_tilt(), -1);
  EXPECT_EQ(sp1.get_flags(), 0);

  supla_action_shading_system_parameters sp2(10, 20,
                                             SSP_FLAG_PERCENTAGE_AS_DELTA);
  EXPECT_EQ(sp2.get_percentage(), 10);
  EXPECT_EQ(sp2.get_tilt(), 20);
  EXPECT_EQ(sp2.get_flags(), SSP_FLAG_PERCENTAGE_AS_DELTA);

  TAction_ShadingSystem_Parameters raw = {};
  raw.Percentage = 50;
  raw.Tilt = 40;
  raw.Flags = SSP_FLAG_TILT_AS_DELTA;

  supla_action_shading_system_parameters sp3(&raw);
  EXPECT_EQ(sp3.get_percentage(), 50);
  EXPECT_EQ(sp3.get_tilt(), 40);
  EXPECT_EQ(sp3.get_flags(), SSP_FLAG_TILT_AS_DELTA);

  cJSON *root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "percentage", 15);
  cJSON_AddNumberToObject(root, "tilt", 20);

  supla_action_shading_system_parameters sp4(root);
  EXPECT_EQ(sp4.get_percentage(), 15);
  EXPECT_EQ(sp4.get_tilt(), 20);
  EXPECT_EQ(sp4.get_flags(), 0);

  cJSON_Delete(root);

  root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "percentageDelta", -15);
  cJSON_AddNumberToObject(root, "tilt", 20);

  supla_action_shading_system_parameters sp5(root);
  EXPECT_EQ(sp5.get_percentage(), -15);
  EXPECT_EQ(sp5.get_tilt(), 20);
  EXPECT_EQ(sp5.get_flags(), SSP_FLAG_PERCENTAGE_AS_DELTA);

  cJSON_Delete(root);

  root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "percentage", 55);
  cJSON_AddNumberToObject(root, "tiltDelta", -20);

  supla_action_shading_system_parameters sp6(root);
  EXPECT_EQ(sp6.get_percentage(), 55);
  EXPECT_EQ(sp6.get_tilt(), -20);
  EXPECT_EQ(sp6.get_flags(), SSP_FLAG_TILT_AS_DELTA);

  cJSON_Delete(root);

  root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "percentageDelta", 55);
  cJSON_AddNumberToObject(root, "tiltDelta", 20);

  supla_action_shading_system_parameters sp7(root);
  EXPECT_EQ(sp7.get_percentage(), 55);
  EXPECT_EQ(sp7.get_tilt(), 20);
  EXPECT_EQ(sp7.get_flags(),
            SSP_FLAG_PERCENTAGE_AS_DELTA | SSP_FLAG_TILT_AS_DELTA);

  cJSON_Delete(root);
}

TEST_F(ActionShadingSystemParametersTest, setAndGetPercentage) {
  supla_action_shading_system_parameters sp;
  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_percentage(-2);
  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_percentage(-101);
  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_flags(SSP_FLAG_TILT_AS_DELTA);

  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_flags(SSP_FLAG_PERCENTAGE_AS_DELTA);

  EXPECT_EQ(sp.get_percentage(), -100);

  sp.set_flags(SSP_FLAG_PERCENTAGE_AS_DELTA | SSP_FLAG_TILT_AS_DELTA);

  EXPECT_EQ(sp.get_percentage(), -100);

  sp.set_flags(0);
  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_percentage(-1);
  EXPECT_EQ(sp.get_percentage(), -1);

  sp.set_percentage(0);
  EXPECT_EQ(sp.get_percentage(), 0);

  sp.set_percentage(100);
  EXPECT_EQ(sp.get_percentage(), 100);

  sp.set_percentage(101);
  EXPECT_EQ(sp.get_percentage(), 100);
}

TEST_F(ActionShadingSystemParametersTest, setAndGetTilt) {
  supla_action_shading_system_parameters sp;
  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_tilt(-2);
  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_tilt(-101);
  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_flags(SSP_FLAG_PERCENTAGE_AS_DELTA);

  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_flags(SSP_FLAG_TILT_AS_DELTA);

  EXPECT_EQ(sp.get_tilt(), -100);

  sp.set_flags(SSP_FLAG_TILT_AS_DELTA | SSP_FLAG_PERCENTAGE_AS_DELTA);

  EXPECT_EQ(sp.get_tilt(), -100);

  sp.set_flags(0);
  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_tilt(-1);
  EXPECT_EQ(sp.get_tilt(), -1);

  sp.set_tilt(0);
  EXPECT_EQ(sp.get_tilt(), 0);

  sp.set_tilt(100);
  EXPECT_EQ(sp.get_tilt(), 100);

  sp.set_tilt(101);
  EXPECT_EQ(sp.get_tilt(), 100);
}

TEST_F(ActionShadingSystemParametersTest, getParams) {
  supla_action_shading_system_parameters sp(10, 20,
                                            SSP_FLAG_PERCENTAGE_AS_DELTA);

  TAction_ShadingSystem_Parameters raw = sp.get_params();

  EXPECT_EQ(raw.Percentage, 10);
  EXPECT_EQ(raw.Tilt, 20);
  EXPECT_EQ(raw.Flags, SSP_FLAG_PERCENTAGE_AS_DELTA);
}

TEST_F(ActionShadingSystemParametersTest, isAnyParamSet) {
  supla_action_shading_system_parameters sp1;
  EXPECT_FALSE(sp1.is_any_param_set());

  supla_action_shading_system_parameters sp2(-1, -1, 0);
  EXPECT_FALSE(sp2.is_any_param_set());

  supla_action_shading_system_parameters sp3(1, -1, 0);
  EXPECT_TRUE(sp3.is_any_param_set());

  supla_action_shading_system_parameters sp4(-1, 1, 0);
  EXPECT_TRUE(sp4.is_any_param_set());

  supla_action_shading_system_parameters sp5(-1, -1,
                                             SSP_FLAG_PERCENTAGE_AS_DELTA);
  EXPECT_TRUE(sp5.is_any_param_set());
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_STOP) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_STOP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 0);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_DOWN) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_DOWN, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 1);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_UP) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_UP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 2);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_DOWN_OR_STOP) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_DOWN_OR_STOP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 0);

  sp.apply_on_value(ACTION_DOWN_OR_STOP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND,
                    SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 3);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_UP_OR_STOP) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_UP_OR_STOP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 0);

  sp.apply_on_value(ACTION_UP_OR_STOP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND,
                    SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 4);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_SBS) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_STEP_BY_STEP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 0);

  sp.apply_on_value(ACTION_STEP_BY_STEP, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND,
                    SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 5);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_REVEAL) {
  supla_action_shading_system_parameters sp;
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  sp.apply_on_value(ACTION_REVEAL, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 10);
}

TEST_F(ActionShadingSystemParametersTest, applyOnValue_Action_SHUT) {
  supla_action_shading_system_parameters sp;
  sp.set_percentage(50);
  sp.set_tilt(25);
  char value[SUPLA_CHANNELVALUE_SIZE] = {};
  ((TDSC_FacadeBlindValue *)value)->position = 100;
  ((TDSC_FacadeBlindValue *)value)->tilt = 100;
  sp.apply_on_value(ACTION_SHUT, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 60);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->tilt, 35);

  ((TDSC_FacadeBlindValue *)value)->position = 15;
  ((TDSC_FacadeBlindValue *)value)->tilt = 26;

  sp.set_flags(SSP_FLAG_PERCENTAGE_AS_DELTA);

  sp.apply_on_value(ACTION_SHUT, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);

  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 75);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->tilt, 35);

  ((TDSC_FacadeBlindValue *)value)->position = 5;
  ((TDSC_FacadeBlindValue *)value)->tilt = 2;

  sp.set_flags(SSP_FLAG_TILT_AS_DELTA);

  sp.apply_on_value(ACTION_SHUT, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);

  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 60);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->tilt, 37);

  ((TDSC_FacadeBlindValue *)value)->position = 1;
  ((TDSC_FacadeBlindValue *)value)->tilt = 3;

  sp.set_flags(SSP_FLAG_PERCENTAGE_AS_DELTA | SSP_FLAG_TILT_AS_DELTA);

  sp.apply_on_value(ACTION_SHUT, value,
                    SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND, 0);

  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->position, 61);
  EXPECT_EQ(((TCSD_FacadeBlindValue *)value)->tilt, 38);
}

} /* namespace testing */
