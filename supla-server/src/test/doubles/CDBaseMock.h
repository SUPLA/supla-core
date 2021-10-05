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

#ifndef CDBASEMOCK_H_
#define CDBASEMOCK_H_

#include "cdbase.h"

class CDBaseMock : public cdbase {
 protected:
  bool db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                       const char Email[SUPLA_EMAIL_MAXSIZE],
                       const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                       database *db);

  int dbAuthCount;

 public:
  explicit CDBaseMock(serverconnection *svrconn);
  int getDbAuthCount();
  void setCacheSizeLimit(int size);
  bool setGUID(char GUID[SUPLA_GUID_SIZE]);
  bool setAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]);

  bool authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                    const char Email[SUPLA_EMAIL_MAXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE]);
};

#endif /* CDBASEMOCK_H_ */
