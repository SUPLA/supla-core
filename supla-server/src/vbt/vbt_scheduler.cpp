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

#include "vbt_scheduler.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "actions/action_executor.h"
#include "device/channel_property_getter.h"
#include "eh.h"
#include "lck.h"
#include "sthread.h"

supla_vbt_scheduler *supla_vbt_scheduler::_global_instance = nullptr;

supla_vbt_scheduler::supla_vbt_scheduler(
    supla_abstract_action_executor *action_executor,
    supla_abstract_channel_property_getter *property_getter) {
  lck = lck_init();
  eh = eh_init();
  sthread = nullptr;
  this->action_executor = action_executor;
  this->property_getter = property_getter;
  sthread_simple_run(loop, this, 0, &sthread);
}

supla_vbt_scheduler::~supla_vbt_scheduler(void) {
  eh_raise_event(eh);
  sthread_twf(sthread, true);
  eh_free(eh);
  lck_free(lck);

  delete action_executor;
  delete property_getter;
}

// static
supla_vbt_scheduler *supla_vbt_scheduler::global_instance(void) {
  if (_global_instance == nullptr) {
    _global_instance = new supla_vbt_scheduler(
        new supla_action_executor(), new supla_channel_property_getter());
  }

  return _global_instance;
}

// static
void supla_vbt_scheduler::loop(void *scheduler, void *sthread) {
  static_cast<supla_vbt_scheduler *>(scheduler)->loop(sthread);
}

void supla_vbt_scheduler::loop(void *sthread) {
  int wait_usec = 0;
  _supla_int64_t time_left = 0;
  double latitude = 0;
  double longitude = 0;
  std::string timezone;
  bool cnd_met = false;

  while (sthread_isterminated(sthread) == 0) {
    eh_wait(eh, wait_usec);
    wait_usec = 1000000;
    std::vector<supla_vbt_condition_result> ready_to_fire;

    lck_lock(lck);

    for (auto it = delayed.begin(); it != delayed.end();) {
      time_left = 0;
      cnd_met = it->get_trigger()->is_condition_met(&time_left);
      time_left *= 1000;

      if (time_left > 0 && time_left < wait_usec) {
        wait_usec = time_left;
      }

      if (time_left <= 0) {
        if (cnd_met) {
          latitude = 0;
          longitude = 0;
          timezone = it->get_user()->get_timezone(&latitude, &longitude);
          if (it->get_trigger()->get_active_period().is_now_active(
                  timezone.c_str(), latitude, longitude)) {
            ready_to_fire.push_back(*it);
          }
        }
        it = delayed.erase(it);
        continue;
      }
      ++it;
    }
    lck_unlock(lck);

    for (auto &item : ready_to_fire) {
      auto template_data = item.get_template_data();
      item.get_trigger()->fire(item.get_caller(), item.get_user()->getUserID(),
                               action_executor, property_getter,
                               &template_data);
    }
  }
}

void supla_vbt_scheduler::erase(
    const std::shared_ptr<supla_value_based_trigger> &trigger) {
  lck_lock(lck);

  for (auto tit = delayed.begin(); tit != delayed.end();) {
    if (tit->get_trigger() == trigger) {
      tit = this->delayed.erase(tit);
      continue;  // Don't break if for some reason there are duplicates on the
                 // list;
    }
    ++tit;
  }

  lck_unlock(lck);

  eh_raise_event(eh);
}

void supla_vbt_scheduler::add_delayed(
    const std::vector<supla_vbt_condition_result> &delayed) {
  lck_lock(lck);
  std::unordered_set<std::shared_ptr<supla_value_based_trigger>> existing;
  existing.reserve(this->delayed.size() + delayed.size());

  for (const auto &item : this->delayed) {
    auto trigger = item.get_trigger();
    existing.insert(trigger);
  }

  for (auto &item : delayed) {
    auto trigger = item.get_trigger();
    auto result = existing.insert(trigger);

    bool inserted = result.second;
    if (inserted) {
      this->delayed.push_back(item);
    }
  }
  lck_unlock(lck);

  eh_raise_event(eh);
}
