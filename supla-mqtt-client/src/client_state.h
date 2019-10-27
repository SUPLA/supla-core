/*
 * client_state.h
 *
 *  Created on: 22 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#ifndef CLIENT_STATE_H_
#define CLIENT_STATE_H_

#include <string>
#include "common.h"
#include "supla-client-lib/proto.h"

class client_state {
 private:
  uint16_t function;
  std::string payload;
  std::string topic;
  bool enabled;

 public:
  client_state();

  uint16_t getFunction();
  std::string getPayload(int id, std::string caption);
  std::string getTopic(int id, int type);
  bool getEnabled();

  void setFunction(std::uint16_t value);
  void setPayload(std::string value);
  void setTopic(std::string value);
  void setEnabled(bool value);
};

#endif /* CLIENT_STATE_H_ */
