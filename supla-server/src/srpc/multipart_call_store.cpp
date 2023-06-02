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

#include "multipart_call_store.h"

supla_multipart_call_store::supla_multipart_call_store(void) {}

supla_multipart_call_store::~supla_multipart_call_store() {
  for (auto it = calls.cbegin(); it != calls.cend(); ++it) {
    delete *it;
  }
}

supla_multipart_call *supla_multipart_call_store::get(unsigned int call_id) {
  for (auto it = calls.begin(); it != calls.end(); ++it) {
    if ((*it)->get_call_id() == call_id) {
      if ((*it)->is_expired()) {
        delete *it;
        calls.erase(it);
        return nullptr;
      }

      return *it;
    }
  }

  return nullptr;
}

void supla_multipart_call_store::release(unsigned int call_id) {
  for (auto it = calls.begin(); it != calls.end(); ++it) {
    if ((*it)->get_call_id() == call_id) {
      delete *it;
      calls.erase(it);
      break;
    }
  }
}

void supla_multipart_call_store::add(supla_multipart_call *call) {
  if (call) {
    release(call->get_call_id());
    calls.push_back(call);
  }
}

size_t supla_multipart_call_store::size(void) { return calls.size(); }
