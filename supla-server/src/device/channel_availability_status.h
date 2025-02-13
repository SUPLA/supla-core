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

#ifndef SUPLA_CHANNEL_AVAILABILITY_STATUS_H_
#define SUPLA_CHANNEL_AVAILABILITY_STATUS_H_

class supla_channel_availability_status {
 private:
  char proto_offline;  // 0 - ONLINE, 1 - OFFLINE - 2 ONLINE BUT NOT AVAILABLE,
                       // 3 - Device doesn't support remote wakeup, so we wait
                       // for it to initiate the communication.
 public:
  supla_channel_availability_status(void);
  supla_channel_availability_status(char status, bool proto_offline);
  explicit supla_channel_availability_status(bool offline);
  virtual ~supla_channel_availability_status(void);

  bool is_online(void) const;
  bool is_offline(void) const;
  bool is_online_but_not_available(void) const;
  bool is_offline_remote_wakeup_not_supported(void) const;

  char get_proto_online(void) const;
  char get_proto_offline(void) const;

  void set_proto_online(char online);
  void set_proto_offline(char offline);

  void set_offline(bool offline);

  bool operator==(const supla_channel_availability_status &status) const;
  bool operator!=(const supla_channel_availability_status &status) const;
};

#endif /* SUPLA_CHANNEL_AVAILABILITY_STATUS_H_ */
