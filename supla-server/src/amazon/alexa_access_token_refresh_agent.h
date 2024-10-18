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

#ifndef ALEXA_ACCESS_TOKEN_REFRESH_AGENT_H_
#define ALEXA_ACCESS_TOKEN_REFRESH_AGENT_H_

#include "cyclictasks/abstract_cyclictask.h"

class supla_alexa_access_token_refresh_agent
    : public supla_abstract_cyclictask {
 protected:
  virtual unsigned int task_interval_sec(void);
  virtual void run(const std::vector<supla_user *> *users,
                   supla_abstract_db_access_provider *dba);
  virtual bool user_access_needed(void);

 public:
  supla_alexa_access_token_refresh_agent();
  virtual ~supla_alexa_access_token_refresh_agent();
};

#endif /* ALEXA_ACCESS_TOKEN_REFRESH_AGENT_H_ */
