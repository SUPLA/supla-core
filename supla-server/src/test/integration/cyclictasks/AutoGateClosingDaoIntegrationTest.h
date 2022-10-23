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

#ifndef AutoGateClosingDaoTest_H_
#define AutoGateClosingDaoTest_H_

#include "cyclictasks/auto_gate_closing_dao.h"
#include "db/db_access_provider.h"
#include "integration/IntegrationTest.h"

namespace testing {

class AutoGateClosingDaoIntegrationTest : public IntegrationTest, public Test {
 protected:
  supla_db_access_provider *dba;
  supla_auto_gate_closing_dao *dao;

 public:
  AutoGateClosingDaoIntegrationTest();
  virtual ~AutoGateClosingDaoIntegrationTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* AutoGateClosingDaoTest_H_ */
