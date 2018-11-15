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

#ifndef TRIVIAL_HTTP_H_
#define TRIVIAL_HTTP_H_

#define HOST_MAXSIZE 1024
#define RESOURCE_MAXSIZE 1024
#define HEADER_MAXSIZE 8192

#include <stdio.h>

class supla_trivial_http {
 protected:
  char *host;
  int port;
  char *resource;
  char *body;
  char *contentType;

  int contentLength;
  int resultCode;

  char *token;

  void set_string_variable(char **var, int max_len, char *src, bool copy);

  virtual ssize_t _write(void *ptr, const void *__buf, size_t __n);
  virtual ssize_t _read(void *ptr, void *__buf, size_t __n);
  virtual bool _should_retry(void *ptr);

  void write_read(void *ptr, const char *out, char **in);
  virtual bool send_recv(const char *out, char **in);
  virtual bool request(const char *method, const char *header,
                       const char *data);

 private:
  bool get_addrinfo(void **res);

  char *header_item_match(const char *item, unsigned int size, const char *name,
                          unsigned int name_size);
  void parse_header_item(const char *item, unsigned int size, bool *chunked);
  bool parse(char **in);
  void releaseResponse(void);

 public:
  supla_trivial_http(const char *host, const char *resource);
  supla_trivial_http(void);
  virtual ~supla_trivial_http(void);
  void setHost(char *host, bool copy = true);
  void setPort(int port);
  void setResource(char *resource, bool copy = true);
  int getResultCode(void);
  int getContentLength(void);
  const char *getContentType(void);
  const char *getBody(void);
  void setToken(char *token, bool copy = true);

  bool http_get(void);
  bool http_post(void);
};

#endif /* TRIVIAL_HTTP_H_ */
