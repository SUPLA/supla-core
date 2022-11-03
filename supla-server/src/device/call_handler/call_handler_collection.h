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

#ifndef SUPLA_DEVICE_CALL_HANDLER_COLLECTION_H_
#define SUPLA_DEVICE_CALL_HANDLER_COLLECTION_H_

#include <memory>

#include "srpc/abstract_srpc_call_hanlder_collection.h"

class supla_device_call_handler_collection
    : public supla_abstract_srpc_call_handler_collection {
 public:
  supla_device_call_handler_collection(void);
  virtual ~supla_device_call_handler_collection();
};

#endif /* SUPLA_DEVICE_CALL_HANDLER_COLLECTION_H_*/
