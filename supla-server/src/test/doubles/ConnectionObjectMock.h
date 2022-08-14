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

#ifndef CONNECTION_OBJECT_MOCK_H_
#define CONNECTION_OBJECT_MOCK_H_

#include <connection_object.h>

class ConnectionObjectMock : public supla_connection_object {
 protected:
  bool db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                       const char Email[SUPLA_EMAIL_MAXSIZE],
                       const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                       database *db);

  int dbAuthCount;

 public:
  explicit ConnectionObjectMock(supla_connection *connection);
  int getDbAuthCount();
  void setCacheSizeLimit(int size);
  bool set_guid(char guid[SUPLA_GUID_SIZE]);
  bool set_authkey(char authkey[SUPLA_AUTHKEY_SIZE]);

  bool authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                    const char Email[SUPLA_EMAIL_MAXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE]);
};

#endif /* CONNECTION_OBJECT_MOCK_H_ */
