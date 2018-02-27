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

#ifndef CDCOMMON_H_
#define CDCOMMON_H_

#include "proto.h"
#include "serverconnection.h"

class supla_user;
class cdcommon {
 private:
  struct timeval last_activity_time;
  char GUID[SUPLA_GUID_SIZE];
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  serverconnection *svrconn;
  int ID;
  supla_user *user;

 protected:
  void *lck;

  // Thread safe start
  bool setGUID(char GUID[SUPLA_GUID_SIZE]);
  bool setAuthKey(char GUID[SUPLA_AUTHKEY_SIZE]);
  void setID(int ID);
  void setUser(supla_user *user);
  // Thread safe end

  serverconnection *getSvrConn(void);

 public:
  explicit cdcommon(serverconnection *svrconn);
  virtual ~cdcommon();

  // Thread safe start
  void terminate(void);
  void getGUID(char GUID[SUPLA_GUID_SIZE]);
  void getAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]);
  int getID(void);
  int getUserID(void);
  supla_user *getUser(void);
  bool cmpGUID(const char GUID1[SUPLA_GUID_SIZE]);
  void updateLastActivity(void);
  int getActivityDelay(void);
  unsigned char getProtocolVersion(void);
  // Thread safe end
};

#endif /* CDCOMMON_H_ */
