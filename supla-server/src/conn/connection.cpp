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

#include "conn/connection.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <serverstatus.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/client.h"
#include "db/database.h"
#include "device/device.h"
#include "log.h"
#include "safearray.h"
#include "srpc/abstract_srpc_call_hanlder_collection.h"
#include "srpc/srpc.h"
#include "srpc/srpc_adapter.h"
#include "sthread.h"
#include "supla-socket.h"
#include "svrcfg.h"
#include "tools.h"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;
using std::weak_ptr;

#define REGISTER_WAIT_TIMEOUT 5
#define REG_NONE 0
#define REG_DEVICE 1
#define REG_CLIENT 2

#define ACTIVITY_TIMEOUT 120
#define UNHANDLED_CALL_MAXCOUNT 5

void *supla_connection::reg_pending_arr = nullptr;
unsigned int supla_connection::local_ipv4[LOCAL_IPV4_ARRAY_SIZE];
struct timeval supla_connection::reg_limit_exceeded_alert_time = {0, 0};
struct timeval supla_connection::reg_limit_exceeded_time = {0, 0};
bool supla_connection::reject_all_new_connections = false;

// static
int supla_connection::socket_read(void *buf, int count, void *sc) {
  return static_cast<supla_connection *>(sc)->socket_read(buf, count);
}

// static
int supla_connection::socket_write(void *buf, int count, void *sc) {
  return static_cast<supla_connection *>(sc)->socket_write(buf, count);
}

// static
void supla_connection::on_remote_call_received(void *_srpc, unsigned int rr_id,
                                               unsigned int call_id, void *sc,
                                               unsigned char proto_version) {
  static_cast<supla_connection *>(sc)->on_remote_call_received(
      _srpc, rr_id, call_id, proto_version);
}

// static
void supla_connection::on_version_error(void *_srpc,
                                        unsigned char remote_version,
                                        void *sc) {
  int a;

  srpc_sdc_async_versionerror(_srpc, remote_version);

  for (a = 0; a < 20; a++) srpc_iterate(_srpc);
}

// static
void supla_connection::log_limits(void) {
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);

  int concurrent_registrations_limit =
      scfg_int(CFG_LIMIT_CONCURRENT_REGISTRATIONS);

  struct timeval now;
  gettimeofday(&now, nullptr);

  if (concurrent_registrations_limit > 0 &&
      supla_connection::registration_pending_count() >=
          concurrent_registrations_limit) {
    if (supla_connection::reg_limit_exceeded_alert_time.tv_sec == 0) {
      supla_log(LOG_ALERT, "Concurrent registration limit exceeded (%i)",
                concurrent_registrations_limit);
      supla_connection::reg_limit_exceeded_alert_time = now;
    } else if (now.tv_sec -
                   supla_connection::reg_limit_exceeded_alert_time.tv_sec >=
               600) {
      supla_log(
          LOG_ALERT,
          "Exceeded number of concurrent registrations takes too long! (%i)",
          concurrent_registrations_limit);
      supla_connection::reg_limit_exceeded_alert_time = now;
      supla_connection::reject_all_new_connections = true;
    }

    supla_connection::reg_limit_exceeded_time = now;

  } else if (supla_connection::reg_limit_exceeded_time.tv_sec &&
             now.tv_sec - supla_connection::reg_limit_exceeded_time.tv_sec >=
                 10) {
    supla_connection::reg_limit_exceeded_time = {0, 0};
    supla_connection::reg_limit_exceeded_alert_time = {0, 0};
    supla_connection::reject_all_new_connections = false;
    supla_log(LOG_INFO,
              "The number of concurrent registrations returned below the "
              "limit");
  }
  serverstatus::globalInstance()->currentLine(__FILE__, __LINE__);
}

// static
bool supla_connection::is_connection_allowed(unsigned int ipv4) {
  if (supla_connection::reject_all_new_connections) {
    //  It also rejects local connections
    //  Monitoring will be able to detect the problem and restart the process
    return false;
  }

  int concurrent_registrations_limit =
      scfg_int(CFG_LIMIT_CONCURRENT_REGISTRATIONS);

  if (concurrent_registrations_limit > 0 &&
      supla_connection::registration_pending_count() >=
          concurrent_registrations_limit) {
    for (int a = 0; a < LOCAL_IPV4_ARRAY_SIZE; a++) {
      if (supla_connection::local_ipv4[a] == 0) {  // end of list
        break;
      } else if (supla_connection::local_ipv4[a] == ipv4) {
        return true;
      }
    }

    return false;
  }

  return true;
}

// static
bool supla_connection::conn_limit_exceeded_hard(void) {
  return supla_connection::reject_all_new_connections;
}

// static
bool supla_connection::conn_limit_exceeded_soft(void) {
  return supla_connection::reg_limit_exceeded_alert_time.tv_sec != 0;
}

// static
void supla_connection::read_local_ipv4_addresses(void) {
  memset(&supla_connection::local_ipv4, 0, sizeof(local_ipv4));

  struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *addr;
  int n = 0;

  if (getifaddrs(&ifaddr) == -1) {
    return;
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa && ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
      addr = (struct sockaddr_in *)ifa->ifa_addr;
      supla_connection::local_ipv4[n] = htonl(addr->sin_addr.s_addr);

      if (supla_connection::local_ipv4[n] != 0) {
        n++;
      }
    }
  }

  freeifaddrs(ifaddr);
}

// static
void supla_connection::init(void) {
  supla_connection::read_local_ipv4_addresses();
  supla_connection::reg_pending_arr = safe_array_init();
}

// static
void supla_connection::cleanup(void) {
  safe_array_free(supla_connection::reg_pending_arr);
}

// static
int supla_connection::registration_pending_count() {
  return safe_array_count(supla_connection::reg_pending_arr);
}

supla_connection::supla_connection(void *ssd, void *supla_socket,
                                   unsigned int client_ipv4) {
  safe_array_add(supla_connection::reg_pending_arr, this);

  gettimeofday(&this->init_time, nullptr);
  this->client_ipv4 = client_ipv4;
  this->sthread = nullptr;
  this->object = nullptr;
  this->ssd = ssd;
  this->supla_socket = supla_socket;
  this->activity_timeout = ACTIVITY_TIMEOUT;
  this->unhandled_call_counter = 0;

  eh = eh_init();
  eh_add_fd(eh, ssocket_supla_socket_getsfd(supla_socket));

  TsrpcParams srpc_params;
  srpc_params_init(&srpc_params);
  srpc_params.user_params = this;
  srpc_params.data_read = &socket_read;
  srpc_params.data_write = &socket_write;
  srpc_params.on_remote_call_received = &on_remote_call_received;
  srpc_params.on_version_error = &on_version_error;
  srpc_params.eh = eh;
  _srpc = srpc_init(&srpc_params);
  srpc_adapter = new supla_srpc_adapter(_srpc);
}

supla_connection::~supla_connection() {
  srpc_free(_srpc);
  delete srpc_adapter;
  eh_free(eh);
  ssocket_supla_socket_free(supla_socket);

  safe_array_remove(supla_connection::reg_pending_arr, this);
  supla_log(LOG_DEBUG, "Connection Finished");
}

void supla_connection::on_object_registration_done(void) {
  safe_array_remove(supla_connection::reg_pending_arr, this);
}

std::shared_ptr<supla_abstract_connection_object> supla_connection::get_object(
    void) {
  return object;
}

int supla_connection::socket_read(void *buf, size_t count) {
  return ssocket_read(ssd, supla_socket, buf, count);
}

int supla_connection::socket_write(const void *buf, size_t count) {
  return ssocket_write(ssd, supla_socket, buf, count);
}

void supla_connection::catch_unhandled_call(unsigned int call_id) {
  unhandled_call_counter++;
  supla_log(LOG_ERR, "Unhandled call %i/%i", call_id, unhandled_call_counter);

  if (unhandled_call_counter >= UNHANDLED_CALL_MAXCOUNT) {
    supla_log(LOG_ERR, "The number of unhandled calls has been exceeded.");
    sthread_terminate(sthread);
  }
}

void supla_connection::on_remote_call_received(void *_srpc, unsigned int rr_id,
                                               unsigned int call_id,
                                               unsigned char proto_version) {
  TsrpcReceivedData rd;

  if (srpc_getdata(_srpc, &rd, rr_id) == SUPLA_RESULT_TRUE) {
    if (object != nullptr) {
      if (object->get_srpc_call_handler_collection()->handle_call(
              object, srpc_adapter, &rd, call_id, proto_version)) {
        object->update_last_activity_time();
      } else {
        catch_unhandled_call(call_id);
      }
    }

    if (object == nullptr && proto_version < SUPLA_PROTO_VERSION &&
        srpc_get_proto_version(_srpc) != proto_version) {
      supla_log(LOG_DEBUG, "Adjusting protocol version to %i", proto_version);
      srpc_adapter->set_proto_version(proto_version);
    }

    if (object == nullptr) {
      object = std::make_shared<supla_device>(this);
      if (!object->get_srpc_call_handler_collection()->handle_call(
              object, srpc_adapter, &rd, call_id, proto_version)) {
        object = nullptr;
      }
    }

    if (object == nullptr) {
      object = std::make_shared<supla_client>(this);
      if (!object->get_srpc_call_handler_collection()->handle_call(
              object, srpc_adapter, &rd, call_id, proto_version)) {
        object = nullptr;
      }
    }

    if (object == nullptr) {
      catch_unhandled_call(call_id);
    }

    srpc_rd_free(&rd);
  }
}

void supla_connection::execute(void *sthread) {
  this->sthread = sthread;

  if (ssocket_accept_ssl(ssd, supla_socket) != 1) {
    sthread_terminate(sthread);
    return;
  }

  supla_log(LOG_DEBUG, "Connection Started %i, secure=%i", sthread,
            ssocket_is_secure(ssd));

  while (sthread_isterminated(sthread) == 0) {
    eh_wait(eh, object == nullptr ? 1000000 : object->wait_time_usec());

    if (srpc_iterate(_srpc) == SUPLA_RESULT_FALSE) {
      // supla_log(LOG_DEBUG, "srpc_iterate(_srpc) == SUPLA_RESULT_FALSE");
      break;
    }

    if (object == nullptr || !object->is_registered()) {
      // TERMINATE IF REGISTRATION TIMEOUT
      struct timeval now;
      gettimeofday(&now, nullptr);

      if (now.tv_sec - init_time.tv_sec >= REGISTER_WAIT_TIMEOUT) {
        sthread_terminate(sthread);
        supla_log(LOG_DEBUG, "Reg timeout", sthread);
        break;
      }

    } else {
      if (object->get_activity_delay() >= get_activity_timeout()) {
        if (object->is_sleeping_object()) {
          supla_log(LOG_DEBUG, "Sleeping device %i", sthread);
        } else {
          sthread_terminate(sthread);
          supla_log(LOG_DEBUG, "Activity timeout %i, %i", sthread,
                    object->get_activity_delay());
        }
        break;
      }
    }
  }

  srpc_adapter->lock();
  ssocket_supla_socket_close(supla_socket);
  srpc_adapter->unlock();

  if (object != nullptr) {
    if (object->is_sleeping_object()) {
      unsigned int time_to_wakeup_msec = object->get_time_to_wakeup_msec();

      if (time_to_wakeup_msec > 0) {
        while (time_to_wakeup_msec > 0 && sthread_isterminated(sthread) == 0) {
          if (time_to_wakeup_msec >= 1000) {
            time_to_wakeup_msec -= 1000;
            usleep(1000000);
          } else {
            usleep(time_to_wakeup_msec * 1000);
            time_to_wakeup_msec = 0;
          }
        }
      }
    }

    weak_ptr<supla_abstract_connection_object> _object = object;
    object = nullptr;

    {
      unsigned long deadlock_counter = 0;

      while (!_object.expired()) {
        usleep(deadlock_counter < 100000 ? 100 : 100000);
        deadlock_counter++;
        if (deadlock_counter == 100000) {
          supla_log(LOG_WARNING,
                    "Too long waiting time to release the object! %x, %i",
                    sthread, _object.use_count());
          break;
        }
      }
    }
  }
}

void supla_connection::terminate(void) { sthread_terminate(sthread); }

unsigned int supla_connection::get_client_ipv4(void) { return client_ipv4; }

int supla_connection::get_client_sd(void) {
  return ssocket_supla_socket_getsfd(supla_socket);
}

supla_abstract_srpc_adapter *supla_connection::get_srpc_adapter(void) {
  return srpc_adapter;
}

void supla_connection::set_activity_timeout(unsigned char timeout) {
  activity_timeout = timeout;
}

unsigned char supla_connection::get_activity_timeout(void) {
  return activity_timeout;
}

unsigned char supla_connection::get_protocol_version(void) {
  return srpc_get_proto_version(_srpc);
}
