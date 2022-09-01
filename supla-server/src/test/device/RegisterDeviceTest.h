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

#ifndef REGISTERDEVICETEST_H_
#define REGISTERDEVICETEST_H_

#include "doubles/SrpcAdapterMock.h"
#include "doubles/conn/AuthkeyCacheStub.h"
#include "doubles/db/DbAccessProviderMock.h"
#include "doubles/device/DeviceDaoMock.h"
#include "doubles/device/RegisterDeviceMock.h"
#include "gtest/gtest.h"

namespace testing {

class RegisterDeviceTest : public Test {
 protected:
  SrpcAdapterMock srpcAdapter;
  DbAccessProviderMock dba;
  DeviceDaoMock dao;
  RegisterDeviceMock rd;
  AuthkeyCacheStub authkeyCache;

  struct timeval setUpTime;
  __useconds_t usecFromSetUp(void);

 public:
  RegisterDeviceTest();
  virtual ~RegisterDeviceTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* REGISTERDEVICETEST_H_ */
