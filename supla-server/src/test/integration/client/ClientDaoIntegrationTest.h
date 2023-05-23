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

#ifndef ClientDaoTest_H_
#define ClientDaoTest_H_

#include "client/client_dao.h"
#include "db/db_access_provider.h"
#include "integration/IntegrationTest.h"

namespace testing {

class ClientDaoIntegrationTest : public IntegrationTest, public Test {
 protected:
  supla_db_access_provider *dba;
  supla_client_dao *dao;

 public:
  ClientDaoIntegrationTest();
  virtual ~ClientDaoIntegrationTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* ClientSceneDaoTest_H_ */
