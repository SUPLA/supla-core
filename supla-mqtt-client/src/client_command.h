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

#ifndef CLIENT_COMMAND_H_
#define CLIENT_COMMAND_H_

#include <string>

class client_command {
 private:
  std::string topic;
  std::string id;
  std::string on_off;
  std::string on_value;
  std::string off_value;
  std::string brightness;
  std::string color_brightness;
  std::string color;
  std::string color_r;
  std::string color_g;
  std::string color_b;
  std::string shut;

 public:
  client_command();

  void setTopic(std::string value);
  void setId(std::string value);
  void setOnOff(std::string value);
  void setOnValue(std::string value);
  void setOffValue(std::string value);
  void setBrightness(std::string value);
  void setColorBrightness(std::string value);
  void setColor(std::string value);
  void setColorR(std::string value);
  void setColorG(std::string value);
  void setColorB(std::string value);
  void setShut(std::string value);

  std::string getTopic();
  std::string getId();
  std::string getOnOff();
  std::string getOnValue();
  std::string getOffValue();
  std::string getBrightness();
  std::string getColorBrghtness();
  std::string getColor();
  std::string getColorR();
  std::string getColorG();
  std::string getColorB();
  std::string getShut();
};
/*
 * on_off 0, 1

brightness 0-100
color_brightness 0 - 100
color '0x0000FF'
color_r 0-255
color_g 0-255
color_b 0-255

shut 0 - 100
 *
 *
 * */

#endif /* CLIENT_COMMAND_H_ */
