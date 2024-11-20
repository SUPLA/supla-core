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

#ifndef CHANNEL_AVAILABILITY_STATUS_H_
#define CHANNEL_AVAILABILITY_STATUS_H_

class channel_availability_status {
 private:
  char proto_offline;  // 0 - ONLINE, 1 - OFFLINE - 2 ONLINE BUT NOT AVAILABLE
 public:
  channel_availability_status(void);
  channel_availability_status(char status, bool proto_offline);
  virtual ~channel_availability_status(void);

  bool is_online(void) const;
  bool is_offline(void) const;
  bool is_online_but_not_available(void) const;

  char get_proto_online(void) const;
  char get_proto_offline(void) const;

  void set_proto_online(char online);
  void set_proto_offline(char offline);

  bool operator==(const channel_availability_status &status) const;
  bool operator!=(const channel_availability_status &status) const;
};

#endif /* CHANNEL_AVAILABILITY_STATUS_H_ */
