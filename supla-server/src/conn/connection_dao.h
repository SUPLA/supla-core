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

#ifndef SUPLA_CONNECTION_DAO_H_
#define SUPLA_CONNECTION_DAO_H_

#include "conn/abstract_connection_dao.h"
#include "svrdb.h"

class supla_connection_dao : public supla_abstract_connection_dao,
                             private svrdb {
 public:
  supla_connection_dao();
  virtual ~supla_connection_dao();

  virtual bool get_reg_enabled(int user_id, unsigned int *client,
                               unsigned int *iodevice);

  virtual bool get_user_localtime(int user_id, TSDC_UserLocalTimeResult *time);
};

#endif /* SUPLA_CONNECTION_DAO_H_ */
