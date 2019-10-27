/*
 * client_command.h
 *
 *  Created on: 15 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
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
