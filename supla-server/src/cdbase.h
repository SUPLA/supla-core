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
class database;
class cdbase {
 private:
  struct timeval last_activity_time;
  char GUID[SUPLA_GUID_SIZE];
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  serverconnection *svrconn;
  int ID;
  unsigned long ptr_counter;
  supla_user *user;

 protected:
  void *lck;
  static void *authkey_auth_cache_arr;
  static int authkey_auth_cache_size;

  // Thread safe start
  bool setGUID(char GUID[SUPLA_GUID_SIZE]);
  bool setAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]);
  void setID(int ID);
  void setUser(supla_user *user);
  // Thread safe end

  serverconnection *getSvrConn(void);
  virtual bool db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                               const char Email[SUPLA_EMAIL_MAXSIZE],
                               const char AuthKey[SUPLA_AUTHKEY_SIZE],
                               int *UserID, database *db) = 0;

  bool authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                    const char Email[SUPLA_EMAIL_MAXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                    database *db);

 public:
  static void init(void);
  static void cdbase_free(void);
  static int getAuthKeyCacheSize(void);
  explicit cdbase(serverconnection *svrconn);
  virtual ~cdbase();
  virtual void iterate();
  virtual unsigned _supla_int64_t waitTimeUSec(void);

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
  cdbase *retainPtr(void);
  void releasePtr(void);
  bool ptrIsUsed(void);
  unsigned long ptrCounter(void);
  // Thread safe end
};

#endif /* CDCOMMON_H_ */
