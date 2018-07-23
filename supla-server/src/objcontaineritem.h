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

#ifndef OBJCONTAINERITEM_H_
#define OBJCONTAINERITEM_H_

#define OI_REMOTEUPDATE_DATATYPE1 0x001
#define OI_REMOTEUPDATE_DATATYPE2 0x002
#define OI_REMOTEUPDATE_DATATYPE3 0x004
#define OI_REMOTEUPDATE_DATATYPE4 0x008

#include "objcontainer.h"
#include "proto.h"

class supla_objcontainer;
class supla_objcontainer_item {
 private:
  supla_objcontainer *Container;
  int Id;

 protected:
 public:
  supla_objcontainer_item(supla_objcontainer *Container, int Id);
  virtual ~supla_objcontainer_item(void);
  supla_objcontainer *getContainer(void);
  int getId();
  virtual int getExtraId();
  virtual void mark_for_remote_update(char mark);
};

#endif /* OBJCONTAINERITEM_H_ */
