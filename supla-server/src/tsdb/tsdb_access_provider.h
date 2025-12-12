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

#ifndef TSDB_ACCESS_PROVIDER_H_
#define TSDB_ACCESS_PROVIDER_H_

#include <pqxx/pqxx>
#include <string>

#include "db/abstract_db_access_provider.h"

class supla_tsdb_access_provider : public supla_abstract_db_access_provider {
 private:
  pqxx::connection* conn = nullptr;
  void append_conninfo_string(std::string& conninfo,
                              const std::string& parameter, const char* value);
  void append_conninfo_string(std::string& conninfo,
                              const std::string& parameter, int value);

 public:
  supla_tsdb_access_provider(void);
  virtual ~supla_tsdb_access_provider(void);
  void log_exception(const std::exception& e);

  virtual bool is_config_present(void);
  virtual bool connect(void);
  virtual bool is_connected(void);
  virtual void disconnect(void);
};

#endif /* TSDB_ACCESS_PROVIDER_H_ */
