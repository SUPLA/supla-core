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

#ifndef GOOGLE_HOME_CREDENTIALS_H_
#define GOOGLE_HOME_CREDENTIALS_H_

#include <list>

#include "http/oauth_credentials.h"

class supla_google_home_credentials : public supla_http_oauth_credentials {
 private:
  std::list<int> excluded_channels;

 public:
  supla_google_home_credentials(void);
  explicit supla_google_home_credentials(supla_user *user);
  ~supla_google_home_credentials(void);

  // These methods are average here, but have been added for convenience
  virtual void exclude_channel(int channel_id);
  virtual bool is_channel_excluded(int channel_id);
  // -------

  virtual void load();
};

#endif /* STATE_WEBHOOK_CREDENTIALS_H_ */
