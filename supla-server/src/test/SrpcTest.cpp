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

#include "SrpcTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "srpc.h"

namespace {

_supla_int_t srpc_data_read(void *buf, _supla_int_t count, void *user_params) {
  return 0;
}

_supla_int_t srpc_data_write(void *buf, _supla_int_t count, void *user_params) {
  return 0;
}

class SrpcTest : public ::testing::Test {
 protected:
  void *srpcInit(void);
};

void *SrpcTest::srpcInit(void) {
  TsrpcParams params;
  srpc_params_init(&params);
  params.data_read = &srpc_data_read;
  params.data_write = &srpc_data_write;

  return srpc_init(&params);
}

TEST_F(SrpcTest, init) {
  void *srpc = srpcInit();
  ASSERT_FALSE(srpc == NULL);
  ASSERT_EQ(SUPLA_PROTO_VERSION, srpc_get_proto_version(srpc));
  srpc_free(srpc);
}

}  // namespace
