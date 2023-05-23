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

#ifndef ABSTRACTACTIONCONFIG_H_
#define ABSTRACTACTIONCONFIG_H_

#include "proto.h"

enum _subjectType_e {
  stUnknown,
  stChannel,
  stChannelGroup,
  stScene,
  stSchedule,
  stPushNotifiction
};

class abstract_action_config {
 public:
  abstract_action_config(void);
  virtual ~abstract_action_config(void);

  virtual int get_action_id(void) = 0;
  virtual _subjectType_e get_subject_type(void) = 0;
  virtual int get_subject_id(void) = 0;

  virtual int get_source_device_id(void) = 0;
  virtual int get_source_channel_id(void) = 0;

  virtual char get_percentage(void) = 0;
  virtual TAction_RGBW_Parameters get_rgbw(void) = 0;
  virtual int get_cap(void);
};

#endif /* ABSTRACTACTIONCONFIG_H_ */
