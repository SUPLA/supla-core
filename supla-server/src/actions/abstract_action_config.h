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

#define ACTION_OPEN 10
#define ACTION_CLOSE 20
#define ACTION_SHUT 30
#define ACTION_REVEAL 40
#define ACTION_REVEAL_PARTIALLY 50
#define ACTION_SHUT_PARTIALLY 51
#define ACTION_TURN_ON 60
#define ACTION_TURN_OFF 70
#define ACTION_SET_RGBW_PARAMETERS 80
#define ACTION_OPEN_CLOSE 90
#define ACTION_STOP 100
#define ACTION_TOGGLE 110
#define ACTION_UP_OR_STOP 140
#define ACTION_DOWN_OR_STOP 150
#define ACTION_STEP_BY_STEP 160
#define ACTION_COPY 10100
#define ACTION_FORWARD_OUTSIDE 10000

typedef struct {
  char brightness;
  char color_brightness;
  unsigned int color;
  bool color_random;
} _action_config_rgbw_t;

enum _subjectType_e { stUnknown, stChannel, stChannelGroup, stScene };

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
  virtual _action_config_rgbw_t get_rgbw(void) = 0;
  virtual int get_cap(void);
};

#endif /* ABSTRACTACTIONCONFIG_H_ */
