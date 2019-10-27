/*
 * client_command.cpp
 *
 *  Created on: 15 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#include "client_command.h"

client_command::client_command() {
  this->topic = "";
  this->id = "";
  this->off_value = "";
  this->on_off = "";
  this->on_value = "";
  this->brightness = "";
  this->color = "";
  this->color_b = "";
  this->color_g = "";
  this->color_r = "";
  this->color_brightness = "";
  this->shut = "";
}

void client_command::setTopic(std::string value) { this->topic = value; }
void client_command::setId(std::string value) { this->id = value; }
void client_command::setOnOff(std::string value) { this->on_off = value; }
void client_command::setOnValue(std::string value) { this->on_value = value; }
void client_command::setOffValue(std::string value) { this->off_value = value; }
void client_command::setBrightness(std::string value) {
  this->brightness = value;
}
void client_command::setColorBrightness(std::string value) {
  this->color_brightness = value;
}
void client_command::setColor(std::string value) { this->color = value; }
void client_command::setColorR(std::string value) { this->color_r = value; }
void client_command::setColorG(std::string value) { this->color_g = value; }
void client_command::setColorB(std::string value) { this->color_b = value; }
void client_command::setShut(std::string value) { this->shut = value; }

std::string client_command::getTopic() { return this->topic; }
std::string client_command::getId() { return this->id; }
std::string client_command::getOnOff() { return this->on_off; }
std::string client_command::getOnValue() { return this->on_value; }
std::string client_command::getOffValue() { return this->off_value; }
std::string client_command::getBrightness() { return this->brightness; }
std::string client_command::getColorBrghtness() {
  return this->color_brightness;
}
std::string client_command::getColor() { return this->color; }
std::string client_command::getColorR() { return this->color_r; }
std::string client_command::getColorG() { return this->color_g; }
std::string client_command::getColorB() { return this->color_b; }
std::string client_command::getShut() { return this->shut; }
