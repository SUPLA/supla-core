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

#include "ProtoTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "proto.h"

namespace {

class ProtoTest : public ::testing::Test {
 protected:
};

TEST_F(ProtoTest, init) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));
  sproto_free(sproto);
}

TEST_F(ProtoTest, set_version) {
  void *sproto = sproto_init();
  ASSERT_FALSE(sproto == NULL);

  sproto_set_version(sproto, SUPLA_PROTO_VERSION_MIN - 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));

  sproto_set_version(sproto, SUPLA_PROTO_VERSION + 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION, sproto_get_version(sproto));

  sproto_set_version(sproto, SUPLA_PROTO_VERSION - 1);
  ASSERT_EQ(SUPLA_PROTO_VERSION - 1, sproto_get_version(sproto));

  sproto_free(sproto);
}

}  // namespace
