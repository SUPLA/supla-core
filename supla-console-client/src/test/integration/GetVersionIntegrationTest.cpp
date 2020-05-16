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

#include "GetVersionIntegrationTest.h"

namespace testing {

GetVersionIntegrationTest::GetVersionIntegrationTest() {
  memset(&version, 0, sizeof(TSDC_SuplaGetVersionResult));
  withoutRegistration = false;
}

GetVersionIntegrationTest::~GetVersionIntegrationTest() {}

void GetVersionIntegrationTest::onConnected() {
  if (withoutRegistration) {
    cancelIteration();
  }
}

void GetVersionIntegrationTest::onRegistered(
    TSC_SuplaRegisterClientResult_B *result) {
  if (!withoutRegistration) {
    cancelIteration();
  }
}

void GetVersionIntegrationTest::onGetVersionResult(
    TSDC_SuplaGetVersionResult *result) {
  ASSERT_FALSE(result == NULL);
  memcpy(&this->version, result, sizeof(TSDC_SuplaGetVersionResult));
  cancelIteration();
}

void GetVersionIntegrationTest::getVersionTest(bool withoutRegistration) {
  this->withoutRegistration = withoutRegistration;
  iterateUntilTimeout(!withoutRegistration, 5000);

  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(supla_client_get_version(sclient), 0);

  iterateUntilTimeout(!withoutRegistration, 5000);

  ASSERT_EQ(version.proto_version_min, 1);
  ASSERT_EQ(version.proto_version, SUPLA_PROTO_VERSION);

  char VersionSuffix[] = "2.3.";
  ASSERT_EQ(memcmp(VersionSuffix, version.SoftVer, strlen(VersionSuffix)), 0);
}

TEST_F(GetVersionIntegrationTest, GetVersionWithoutRegistration) {
  getVersionTest(true);
}

TEST_F(GetVersionIntegrationTest, GetVersionWithRegistration) {
  getVersionTest(false);
}

} /* namespace testing */
