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

#ifndef USER_CLIENTCONTAINER_H_
#define USER_CLIENTCONTAINER_H_

#include "cdcontainer.h"

class supla_user_client_container : public cdcontainer {
 private:
  static char find_client_byid(void *ptr, void *ID);
  static char find_client_byguid(void *ptr, void *GUID);

  supla_client *baseToClient(cdbase *base);

 protected:
  virtual void cd_delete(cdbase *base);

 public:
  supla_user_client_container();
  virtual ~supla_user_client_container();

  supla_client *findByID(int ClientID);
  supla_client *findByGUID(const char *GUID);
  supla_client *get(int idx);
};

#endif /* USER_CLIENTCONTAINER_H_ */
