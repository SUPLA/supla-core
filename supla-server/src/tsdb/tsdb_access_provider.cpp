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

#include "log.h"
#include "svrcfg.h"

using std::string;

supla_tsdb_access_provider::supla_tsdb_access_provider(void)
    : supla_abstract_db_access_provider() {
  conn = nullptr;
}

supla_tsdb_access_provider::~supla_tsdb_access_provider(void) { disconnect(); }

void supla_tsdb_access_provider::append_conninfo_string(string& conninfo,
                                                        const string& parameter,
                                                        const char* value) {
  if (value) {
    size_t len = strnlen(value, 1000);

    conninfo.append(parameter);
    conninfo.append("=\'");

    string esc;
    for (size_t n = 0; n < len; n++) {
      if (value[n] == '\'' || value[n] == '\\') {
        esc.push_back('\\');
      }
      esc.push_back(value[n]);
    }

    conninfo.append(esc);

    conninfo.append("\' ");
  }
}

void supla_tsdb_access_provider::append_conninfo_string(string& conninfo,
                                                        const string& parameter,
                                                        int value) {
  conninfo.append(parameter);
  conninfo.append("=");
  conninfo.append(std::to_string(value));
  conninfo.append(" ");
}

void supla_tsdb_access_provider::log_exception(const std::exception& e) {
  supla_log(LOG_ERR, "TSDB: %s", e.what());
}

bool supla_tsdb_access_provider::connect(void) {
  if (is_connected()) {
    return true;
  }

  if (!is_config_present()) {
    return false;
  }

  string conninfo;

  append_conninfo_string(conninfo, "host", scfg_string(CFG_TSDB_HOST));
  append_conninfo_string(conninfo, "port", scfg_int(CFG_TSDB_PORT));
  append_conninfo_string(conninfo, "dbname", scfg_string(CFG_TSDB_DB));
  append_conninfo_string(conninfo, "user", scfg_string(CFG_TSDB_USER));
  append_conninfo_string(conninfo, "password", scfg_string(CFG_TSDB_PASSWORD));

  if (conn) {
    delete conn;
  }

  disconnect();

  try {
    conn = new pqxx::connection(conninfo);
  } catch (const std::exception& e) {
    if (conn) {
      delete conn;
      conn = nullptr;
    }

    log_exception(e);
  }

  return is_connected();
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
  return strnlen(scfg_string(CFG_TSDB_HOST), 3) > 0 &&
         strnlen(scfg_string(CFG_TSDB_DB), 3) > 0 &&
         strnlen(scfg_string(CFG_TSDB_USER), 3) > 0;
}
