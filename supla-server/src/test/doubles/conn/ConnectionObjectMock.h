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

#include <gmock/gmock.h>

#include "conn/connection_object.h"

class ConnectionObjectMock : public supla_connection_object {
 public:
  explicit ConnectionObjectMock(supla_connection *connection);
  void set_cache_size_limit(int size);
  void set_id(int id);
  bool set_guid(const char guid[SUPLA_GUID_SIZE]);
  bool set_authkey(const char authkey[SUPLA_AUTHKEY_SIZE]);

  bool authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                    const char Email[SUPLA_EMAIL_MAXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE]);

  virtual bool is_sleeping_object(void);
  virtual unsigned int get_time_to_wakeup_msec(void);

  MOCK_METHOD5(db_authkey_auth, bool(const char guid[SUPLA_GUID_SIZE],
                                     const char email[SUPLA_EMAIL_MAXSIZE],
                                     const char authkey[SUPLA_AUTHKEY_SIZE],
                                     int *user_id, database *db));
};

#endif /* CONNECTION_OBJECT_MOCK_H_ */
