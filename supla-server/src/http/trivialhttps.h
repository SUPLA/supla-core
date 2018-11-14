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

#include <http/trivialhttp.h>

#ifndef TRIVIAL_HTTPS_H_
#define TRIVIAL_HTTPS_H_
#ifndef NOSSL

class supla_trivial_https : public supla_trivial_http {
 private:
  void *ssl_vars;
  void vars_init(void);
  void vars_free(void);
  void log_ssl_error(void);
  static char *caFile;

 protected:
  virtual bool send_recv(const char *out, char **in);
  virtual ssize_t _write(void *ptr, const void *__buf, size_t __n);
  virtual ssize_t _read(void *ptr, void *__buf, size_t __n);
  virtual bool _should_retry(void *ptr);

 public:
  static void init(void);
  supla_trivial_https(const char *host, const char *resource);
  supla_trivial_https(void);
  ~supla_trivial_https(void);
};

#endif /* NOSSL */
#endif /* TRIVIAL_HTTPS_H_ */
