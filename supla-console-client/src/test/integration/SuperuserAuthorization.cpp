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

#include "SuperuserAuthorization.h"

namespace testing {

SuperuserAuthorization::SuperuserAuthorization() {}

SuperuserAuthorization::~SuperuserAuthorization() {}

void SuperuserAuthorization::onSuperuserAuthorizationResult(bool authorized,
                                                            int code) {
  ASSERT_TRUE(authorized);
  ASSERT_EQ(code, SUPLA_RESULTCODE_AUTHORIZED);
  cancelIteration();
}

void SuperuserAuthorization::superuserAuthorize() {
  char email[] = TESTUSER_EMAIL;
  char password[] = TESTUSER_PASSWD;
  ASSERT_FALSE(sclient == NULL);
  ASSERT_GT(
      supla_client_superuser_authorization_request(sclient, email, password),
      0);

  iterateUntilDefaultTimeout();
}

} /* namespace testing */
