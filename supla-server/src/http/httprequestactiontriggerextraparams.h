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

#ifndef HTTP_HTTPREQUESTACTIONTRIGGEREXTRAPARAMS_H_
#define HTTP_HTTPREQUESTACTIONTRIGGEREXTRAPARAMS_H_

#include "http/httprequestextraparams.h"

class supla_http_request_action_trigger_extra_params
    : public supla_http_request_extra_params {
 private:
  unsigned int actions;

 public:
  supla_http_request_action_trigger_extra_params(void);
  explicit supla_http_request_action_trigger_extra_params(unsigned int actions);
  unsigned int getActions(void);
  void addActions(unsigned int actions);
};

#endif /* HTTP_HTTPREQUESTACTIONTRIGGEREXTRAPARAMS_H_ */
