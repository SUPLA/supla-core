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

#ifndef TSDB_PowerActiveLoggerDaoTest_H_
#define TSDB_PowerActiveLoggerDaoTest_H_

#include "datalogger/power_active_logger_dao.h"
#include "integration/datalogger/TSDB_LoggerDaoIntegrationTest.h"

namespace testing {

class TSDB_PowerActiveLoggerDaoIntegrationTest
    : public TSDB_LoggerDaoIntegrationTest {
 protected:
  supla_power_active_logger_dao *dao;

 public:
  TSDB_PowerActiveLoggerDaoIntegrationTest();
  virtual ~TSDB_PowerActiveLoggerDaoIntegrationTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* TSDB_PowerActiveLoggerDaoTest_H_ */
