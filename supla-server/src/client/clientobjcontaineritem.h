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
#define OI_REMOTEUPDATE_DATA1 0x01
#define OI_REMOTEUPDATE_DATA2 0x02
#define OI_REMOTEUPDATE_DATA3 0x04
#define OI_REMOTEUPDATE_DATA4 0x08

#include "proto.h"
#include "objcontaineritem.h"

class supla_client;
class supla_client_objcontainer;
class supla_client_objcontainer_item : public supla_objcontainer_item {
 private:
  char *Caption;


 protected:
  void proto_get_caption(char *Caption, unsigned _supla_int_t *CaptionSize,
                         unsigned int MaxSize);

 public:
  explicit supla_client_objcontainer_item(supla_client_objcontainer *Container,
                                          int Id, const char *Caption);
  virtual ~supla_client_objcontainer_item(void);
  supla_client_objcontainer *getContainer(void);
  char *getCaption(void);
  void setCaption(const char *Caption);

};

#endif /* CLIENT_CLIENTOBJCONTAINERITEM_H_ */
