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

#ifndef ACTION_EXECUTOR_MOCK_H_
#define ACTION_EXECUTOR_MOCK_H_

#include "actions/abstract_channel_action_executor.h"

namespace testing {

class ChannelActionExecutorMock
    : public supla_abstract_channel_action_executor {
 private:
  int on_counter;
  int color_counter;
  int brightness_counter;
  int color_brightness_counter;
  int off_counter;
  int toggle_counter;
  int shut_counter;
  int reveal_counter;
  int stop_counter;
  int open_counter;
  int close_counter;
  int open_close_counter;
  int open_close_wct_counter;

  unsigned int color;
  char brightness;
  char color_brightness;
  char closing_percentage;

 public:
  ChannelActionExecutorMock();
  virtual ~ChannelActionExecutorMock();

  virtual void set_on(bool on);
  virtual void set_color(unsigned int color);
  virtual void set_brightness(char brightness);
  virtual void set_color_brightness(char brightness);
  virtual void toggle(void);
  virtual void shut(const char *closingPercentage);
  virtual void reveal(void);
  virtual void stop(void);
  virtual void open(void);
  virtual void close(void);
  virtual void open_close(void);
  virtual void open_close_without_canceling_tasks(void);

  void clear(void);
  int counterSetCount(void);
  int getOnCounter(void);
  int getOffCounter(void);
  int getColorCounter(void);
  int getBrightnessCounter(void);
  int getColorBrightnessCounter(void);
  int getToggleCounter(void);
  int getShutCounter(void);
  int getRevealCounter(void);
  int getStopCounter(void);
  int getOpenCounter(void);
  int getCloseCounter(void);
  int getOpenCloseCounter(void);
  int getOpenCloseWctCounter(void);
  char getClosingPercentage(void);
  unsigned int getColor(void);
  char getBrightness(void);
  char getColorBrightness(void);
};

} /* namespace testing */

#endif /* ACTION_EXECUTOR_MOCK_H_ */
