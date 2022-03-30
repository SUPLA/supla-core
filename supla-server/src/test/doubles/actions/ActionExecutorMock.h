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

#include <actions/abstract_action_executor.h>

namespace testing {

class ActionExecutorMock : public supla_abstract_action_executor {
 private:
  int on_counter;
  int color_counter;
  int brightness_counter;
  int color_brightness_counter;
  int off_counter;
  int toggle_counter;
  int shut_counter;
  int reveal_counter;
  int up_counter;
  int down_counter;
  int up_or_stop_counter;
  int down_or_stop_counter;
  int step_by_step_counter;
  int start_counter;
  int stop_counter;
  int open_counter;
  int close_counter;
  int open_close_counter;
  int open_close_wct_counter;
  int rgbw_counter;
  int forward_outside_counter;
  unsigned int color;
  char brightness;
  char color_brightness;
  char closing_percentage;
  char rgbw_on_off;

 public:
  ActionExecutorMock();
  virtual ~ActionExecutorMock();
  void access_device(std::function<void(supla_device *device)> on_device);

  virtual void set_on(bool on);
  virtual void set_color(unsigned int color);
  virtual void set_brightness(char brightness);
  virtual void set_color_brightness(char brightness);
  virtual void set_rgbw(unsigned int *color, char *color_brightness,
                        char *brightness, char *on_off);
  virtual void toggle(void);
  virtual void shut(const char *closingPercentage);
  virtual void reveal(void);
  virtual void up(void);
  virtual void down(void);
  virtual void up_or_stop(void);
  virtual void down_or_stop(void);
  virtual void step_by_step(void);
  virtual void start(void);
  virtual void stop(void);
  virtual void open(void);
  virtual void close(void);
  virtual void open_close(void);
  virtual void open_close_without_canceling_tasks(void);
  virtual void forward_outside(int cap);

  void clear(void);
  int counterSetCount(void);
  int getOnCounter(void);
  int getOffCounter(void);
  int getColorCounter(void);
  int getBrightnessCounter(void);
  int getColorBrightnessCounter(void);
  int getRGBWCounter(void);
  int getToggleCounter(void);
  int getShutCounter(void);
  int getRevealCounter(void);
  int getUpCounter(void);
  int getDownCounter(void);
  int getUpOrStopCounter(void);
  int getDownOrStopCounter(void);
  int getStepByStepCounter(void);
  int getStopCounter(void);
  int getStartCounter(void);
  int getOpenCounter(void);
  int getCloseCounter(void);
  int getOpenCloseCounter(void);
  int getOpenCloseWctCounter(void);
  int getForwardOutsideCounter(void);
  char getClosingPercentage(void);
  unsigned int getColor(void);
  char getBrightness(void);
  char getColorBrightness(void);
  char getRGBWOnOff(void);
};

} /* namespace testing */

#endif /* ACTION_EXECUTOR_MOCK_H_ */
