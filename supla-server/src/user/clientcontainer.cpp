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

#include <clientcontainer.h>
#include "client/client.h"

// static
char supla_user_client_container::find_client_byid(void *ptr, void *ID) {
  return ((supla_client *)ptr)->getID() == *(int *)ID ? 1 : 0;
}

// static
char supla_user_client_container::find_client_byguid(void *ptr, void *GUID) {
  return ((supla_client *)ptr)->cmpGUID((char *)GUID) ? 1 : 0;
}

supla_user_client_container::supla_user_client_container() : cdcontainer() {}

supla_user_client_container::~supla_user_client_container() {}

void supla_user_client_container::cd_delete(cdbase *base) {
  supla_client *client = static_cast<supla_client *>(base);
  if (client) {
    delete client;
  }
}

supla_client *supla_user_client_container::baseToClient(cdbase *base) {
  supla_client *client = NULL;
  if (base && (client = static_cast<supla_client *>(base)) == NULL) {
    base->releasePtr();
  }
  return client;
}

supla_client *supla_user_client_container::findByID(int ClientID) {
  if (ClientID == 0) {
    return NULL;
  }
  return baseToClient(find(find_client_byid, &ClientID));
}

supla_client *supla_user_client_container::findByGUID(const char *GUID) {
  return baseToClient(find(find_client_byguid, (void *)GUID));
}

supla_client *supla_user_client_container::get(int idx) {
  return static_cast<supla_client *>(cdcontainer::get(idx));
}
