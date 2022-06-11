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

#ifndef DISTRIBUTED_OBJECT_REMOTE_UPDATER_MOCK_H_
#define DISTRIBUTED_OBJECT_REMOTE_UPDATER_MOCK_H_

#include <gmock/gmock.h>

#include "distributedobjects/abstract_dobject_remote_updater.h"

namespace testing {

class DObjectRemoteUpdaterMock : public supla_abstract_dobject_remote_updater {
 private:
 protected:
 public:
  explicit DObjectRemoteUpdaterMock(supla_abstract_srpc_adapter *srpc_adapter);
  virtual ~DObjectRemoteUpdaterMock(void);
  MOCK_METHOD1(is_protocol_version_allowed, bool(int protocol_version));
  MOCK_METHOD2(on_transaction_begin,
               bool(supla_dobject *object, int protocol_version));
  MOCK_METHOD1(on_transaction_end,
               void(supla_abstract_srpc_adapter *srpc_adapter));
  MOCK_METHOD3(prepare_the_update,
               bool(supla_dobject *object,
                    supla_dobject_change_indicator **new_change_indicator,
                    bool *transaction_should_end));
};

} /* namespace testing */

#endif /* DISTRIBUTED_OBJECT_REMOTE_UPDATER_MOCK_H_ */
