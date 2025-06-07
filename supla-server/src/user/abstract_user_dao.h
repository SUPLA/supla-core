
#ifndef SUPLA_ABSTRACT_USER_DAO_H_
#define SUPLA_ABSTRACT_USER_DAO_H_

#include <vector>

#include "user/virtualchannel.h"

class supla_user;
class supla_abstract_user_dao {
 public:
  supla_abstract_user_dao();
  virtual ~supla_abstract_user_dao();

  virtual std::vector<supla_virtual_channel> get_virtual_channels(
      supla_user *user, long long int since_the_value_update_timestamp) = 0;
  virtual std::vector<int> get_users_with_virtual_channels_online(void) = 0;
};

#endif /*SUPLA_ABSTRACT_USER_DAO_H_*/
