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

#ifndef SUPLA_CLIENT_DAO_H_
#define SUPLA_CLIENT_DAO_H_

#include "client/abstract_client_dao.h"
#include "svrdb.h"

class supla_client_dao : public supla_abstract_client_dao, private svrdb {
 private:
  int oauth_add_client_id(void);
  int oauth_get_client_id(bool create);

 public:
  supla_client_dao();
  virtual ~supla_client_dao();

  virtual bool oauth_get_token(TSC_OAuthToken *token, int user_id,
                               int access_id, bool *storage_connect_error);
};

#endif /* SUPLA_CLIENT_DAO_H_ */
