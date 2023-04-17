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

#include "abstract_access_token_provider.h"

#include <unistd.h>

#include "http/curl_adapter.h"
#include "lck.h"
#include "sthread.h"

using std::string;

supla_abstract_access_token_provider::supla_abstract_access_token_provider(
    void) {
  curl_adapter = nullptr;
  refresh_lck = lck_init();
  data_lck = lck_init();
  last_refresh_attpemt_time = {};
  expires_at = {};
  thread = nullptr;
}

supla_abstract_access_token_provider::~supla_abstract_access_token_provider(
    void) {
  stop_service();

  lck_free(data_lck);
  lck_free(refresh_lck);
}

long long supla_abstract_access_token_provider::service_tick_time_usec(void) {
  return 1000000;
}

// static
void supla_abstract_access_token_provider::_service_loop(void *_provider,
                                                         void *sthread) {
  supla_abstract_access_token_provider *provider =
      static_cast<supla_abstract_access_token_provider *>(_provider);
  if (!provider) {
    return;
  }

  provider->thread_init();

  while (sthread_isterminated(sthread) == 0) {
    provider->service_loop();
    usleep(provider->service_tick_time_usec());
  }

  provider->thread_cleanup();
}

void supla_abstract_access_token_provider::thread_init(void) {}

void supla_abstract_access_token_provider::thread_cleanup(void) {}

void supla_abstract_access_token_provider::service_loop(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  lck_lock(data_lck);
  long long secs_left = expires_at.tv_sec - now.tv_sec;
  lck_unlock(data_lck);

  secs_left -= refresh_time_margin_secs();

  if (secs_left <= 0) {
    refresh(curl_adapter);
  }
}

void supla_abstract_access_token_provider::start_service(
    supla_abstract_curl_adapter *curl_adapter) {
  if (thread) {
    if (curl_adapter) {
      delete curl_adapter;
    }
    return;
  }

  if (this->curl_adapter) {
    delete this->curl_adapter;
  }

  this->curl_adapter = curl_adapter;

  sthread_simple_run(_service_loop, this, 0, &thread);
}

void supla_abstract_access_token_provider::start_service(void) {
  start_service(new supla_curl_adapter());
}

void supla_abstract_access_token_provider::stop_service(void) {
  if (thread) {
    sthread_twf(thread, false);
    thread = nullptr;
  }

  if (curl_adapter) {
    delete curl_adapter;
    curl_adapter = nullptr;
  }
}

int supla_abstract_access_token_provider::refresh_time_margin_secs(void) {
  return 300;
}

int supla_abstract_access_token_provider::min_secs_between_refresh_attempts(
    void) {
  return 60;
}

bool supla_abstract_access_token_provider::refresh(
    supla_abstract_curl_adapter *curl_adapter) {
  bool result = false;
  int expires_in_secs = 0;
  string token;

  lck_lock(refresh_lck);
  struct timeval now = {};
  gettimeofday(&now, nullptr);
  if (last_refresh_attpemt_time.tv_sec == 0 ||
      now.tv_sec - last_refresh_attpemt_time.tv_sec >=
          min_secs_between_refresh_attempts()) {
    last_refresh_attpemt_time = now;
    result = new_token(curl_adapter, &token, &expires_in_secs);
  }
  lck_unlock(refresh_lck);

  if (result) {
    lck_lock(data_lck);

    this->token = token;

    if (!expires_in_secs) {
      expires_in_secs = 60 * 60 * 24 * 365 * 10;  // 10 years
    }

    gettimeofday(&expires_at, nullptr);
    expires_at.tv_sec += expires_in_secs;

    lck_unlock(data_lck);
  }

  return result;
}

bool supla_abstract_access_token_provider::refresh(void) {
  return refresh(nullptr);
}

string supla_abstract_access_token_provider::get_token(void) {
  lck_lock(data_lck);
  string result = token;
  lck_unlock(data_lck);

  return result;
}

bool supla_abstract_access_token_provider::is_token_valid(void) {
  bool result = false;

  lck_lock(data_lck);
  if (token.size()) {
    struct timeval now = {};
    gettimeofday(&now, nullptr);
    result = now.tv_sec < expires_at.tv_sec;
  }
  lck_unlock(data_lck);

  return result;
}

bool supla_abstract_access_token_provider::is_service_running(void) {
  return thread != nullptr;
}
