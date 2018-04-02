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

#ifndef CLIENT_CLIENTOBJCONTAINERITEM_H_
#define CLIENT_CLIENTOBJCONTAINERITEM_H_

#define OI_REMOTEUPDATE_NONE 0
#define OI_REMOTEUPDATE_FULL 1
#define OI_REMOTEUPDATE_VALUE 2

class supla_client;
class supla_client_objcontainer_item {
 private:
  int Id;
  char *Caption;
  char RemoteUpdateMark;

 protected:
  virtual bool remote_update_is_possible(void) = 0;

 public:
  explicit supla_client_objcontainer_item(int Id, const char *Caption);
  explicit supla_client_objcontainer_item(supla_client_objcontainer_item *obj);
  virtual ~supla_client_objcontainer_item(void);
  virtual void update(supla_client_objcontainer_item *item);
  int getId();
  virtual int getExtraId();
  char *getCaption(void);
  void setCaption(const char *Caption);

  void mark_for_remote_update(char mark);
  char marked_for_remote_update(void);
};

#endif /* CLIENT_CLIENTOBJCONTAINERITEM_H_ */
