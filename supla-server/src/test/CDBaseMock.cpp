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

#include "CDBaseMock.h"   // NOLINT
#include "gtest/gtest.h"  // NOLINT

CDBaseMock::CDBaseMock(serverconnection *svrconn) : cdbase(svrconn) {
  dbAuthCount = 0;
}

bool CDBaseMock::db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                                 const char Email[SUPLA_EMAIL_MAXSIZE],
                                 const char AuthKey[SUPLA_AUTHKEY_SIZE],
                                 int *UserID, database *db) {
  dbAuthCount++;
  return dbAuthCount > 1;
}

int CDBaseMock::getDbAuthCount() { return dbAuthCount; }

void CDBaseMock::setCacheSizeLimit(int size) { authkey_auth_cache_size = size; }

bool CDBaseMock::authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                              const char Email[SUPLA_EMAIL_MAXSIZE],
                              const char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  int UserID = 0;
  return cdbase::authkey_auth(GUID, Email, AuthKey, &UserID, NULL);
}

bool CDBaseMock::setGUID(char GUID[SUPLA_GUID_SIZE]) {
  return cdbase::setGUID(GUID);
}

bool CDBaseMock::setAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  return cdbase::setAuthKey(AuthKey);
}
