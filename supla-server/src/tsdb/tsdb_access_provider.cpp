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

#include "tsdb_access_provider.h"

#include <string.h>

#include <atomic>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <string>

#include "log.h"
#include "svrcfg.h"

using pqxx::connection;
using pqxx::nontransaction;
using pqxx::result;
using std::atomic;
using std::string;

supla_tsdb_access_provider::supla_tsdb_access_provider(void)
    : supla_abstract_db_access_provider() {
  conn = nullptr;
}

supla_tsdb_access_provider::supla_tsdb_access_provider(
    const std::string& extra_conn_args) {
  this->extra_conn_args = extra_conn_args;
}

supla_tsdb_access_provider::~supla_tsdb_access_provider(void) { disconnect(); }

void supla_tsdb_access_provider::append_conninfo_string(string* conninfo,
                                                        const string& parameter,
                                                        const char* value,
                                                        bool add_apostrophes) {
  if (value) {
    size_t len = strnlen(value, 1000);

    conninfo->append(parameter);
    conninfo->append("=");

    if (add_apostrophes) {
      conninfo->append("\'");

      string esc;
      for (size_t n = 0; n < len; n++) {
        if (value[n] == '\'' || value[n] == '\\') {
          esc.push_back('\\');
        }
        esc.push_back(value[n]);
      }

      conninfo->append(esc);

      conninfo->append("\'");
    } else {
      conninfo->append(value);
    }

    conninfo->append(" ");
  }
}

void supla_tsdb_access_provider::append_conninfo_string(string* conninfo,
                                                        const string& parameter,
                                                        int value) {
  conninfo->append(parameter);
  conninfo->append("=");
  conninfo->append(std::to_string(value));
  conninfo->append(" ");
}

void supla_tsdb_access_provider::log_exception(const std::exception& e,
                                               int channel_id) {
  if (channel_id) {
    supla_log(LOG_ERR, "TSDB (channel_id %i): %s", channel_id, e.what());
  } else {
    supla_log(LOG_ERR, "TSDB: %s", e.what());
  }
}

bool supla_tsdb_access_provider::connect(void) {
  if (is_connected()) {
    return true;
  }

  if (!is_config_present()) {
    return false;
  }

  string conninfo;

  append_conninfo_string(&conninfo, "host", scfg_string(CFG_TSDB_HOST), true);
  append_conninfo_string(&conninfo, "port", scfg_int(CFG_TSDB_PORT));
  append_conninfo_string(&conninfo, "dbname", scfg_string(CFG_TSDB_DB), true);
  append_conninfo_string(&conninfo, "user", scfg_string(CFG_TSDB_USER), true);
  append_conninfo_string(&conninfo, "password", scfg_string(CFG_TSDB_PASSWORD),
                         true);

  append_conninfo_string(&conninfo, "sslmode", scfg_string(CFG_TSDB_SSLMODE),
                         false);
  append_conninfo_string(&conninfo, "sslrootcert",
                         scfg_string(CFG_TSDB_SSLROOTCERT), false);
  append_conninfo_string(&conninfo, "sslcert", scfg_string(CFG_TSDB_SSLCERT),
                         false);
  append_conninfo_string(&conninfo, "sslkey", scfg_string(CFG_TSDB_SSLKEY),
                         false);

  if (extra_conn_args.length()) {
    conninfo.append(" ");
    conninfo.append(extra_conn_args);
  }

  disconnect();

  try {
    conn = new connection(conninfo);

    nontransaction ntx(*conn);
    ntx.exec("SET TIME ZONE 'UTC'");
  } catch (const std::exception& e) {
    log_exception(e);
    disconnect();
  }

  if (is_connected()) {
    if (version_ok.load(std::memory_order_acquire)) {
      return true;
    } else if (check_db_version()) {
      version_ok.store(true, std::memory_order_release);
      return true;
    } else {
      disconnect();
    }
  }

  return false;
}

bool supla_tsdb_access_provider::is_connected(void) {
  return conn && conn->is_open();
}

void supla_tsdb_access_provider::disconnect(void) {
  if (conn) {
    delete conn;
    conn = nullptr;
  }
}

bool supla_tsdb_access_provider::is_config_present(void) {
  const char* host = scfg_string(CFG_TSDB_HOST);
  const char* db = scfg_string(CFG_TSDB_DB);
  const char* user = scfg_string(CFG_TSDB_USER);

  return host && db && user && strnlen(host, 3) > 0 && strnlen(db, 3) > 0 &&
         strnlen(user, 3) > 0;
}

connection* supla_tsdb_access_provider::get_conn(void) { return conn; }

string supla_tsdb_access_provider::time_to_timestamp_string(const time_t& t) {
  std::tm tm{};
  gmtime_r(&t, &tm);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

bool supla_tsdb_access_provider::check_db_version(void) {
  string found;
  string expected = "20250625101351";

  bool _is_connected = is_connected();
  if (!_is_connected && !connect()) {
    return false;
  }

  try {
    nontransaction ntx(*conn);

    result r = ntx.exec(
        "SELECT MAX(SUBSTRING(version FROM POSITION('Version' IN version) + "
        "7)) FROM doctrine_migration_versions");

    if (!r.empty() && !r[0][0].is_null()) {
      found = r[0][0].as<string>();
    }
  } catch (const std::exception& e) {
    log_exception(e);
  }

  if (!_is_connected) {
    disconnect();
  }

  if (found.empty()) {
    supla_log(LOG_ERR,
              "The version of the Timescale database can not be determined!");
  } else if (found != expected) {
    supla_log(LOG_ERR,
              "Incorrect Timescale database version! Expected: %s, Found: %s",
              expected.c_str(), found.c_str());
  } else {
    return true;
  }

  return false;
}
