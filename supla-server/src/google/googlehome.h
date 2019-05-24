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

#ifndef GOOGLEHOME_H_
#define GOOGLEHOME_H_

#define GH_TOKEN_MAXSIZE 255

#include <voiceassistant.h>

class supla_google_home : public supla_voice_assistant {
 protected:
  virtual int get_token_maxsize(void);
  int sync_40x_counter;

 public:
  explicit supla_google_home(supla_user *user);
  void load();
  void on_credentials_changed();
  void on_sync_40x_error();
  void on_reportstate_404_error();
};

#endif /* GOOGLEHOME_H_ */
