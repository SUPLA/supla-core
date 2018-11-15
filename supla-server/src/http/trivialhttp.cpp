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

#define OUTDATA_MAXSIZE 102400
#define INDATA_MAXSIZE 102400
#define METHOD_MAXSIZE 20
#define IN_BUFFER_SIZE 1024
#define TOKEN_MAXSIZE 2048

#include <arpa/inet.h>
#include <http/trivialhttp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "log.h"

supla_trivial_http::supla_trivial_http(const char *host, const char *resource) {
  this->resultCode = 0;
  this->contentLength = 0;
  this->contentType = NULL;
  this->port = 80;
  this->host = host ? strndup(host, HOST_MAXSIZE) : NULL;
  this->resource = resource ? strndup(resource, RESOURCE_MAXSIZE) : NULL;
  this->body = NULL;
  this->token = NULL;
}

supla_trivial_http::supla_trivial_http(void) {
  this->resultCode = 0;
  this->contentLength = 0;
  this->contentType = NULL;
  this->port = 80;
  this->host = NULL;
  this->resource = NULL;
  this->body = NULL;
  this->token = NULL;
}

supla_trivial_http::~supla_trivial_http(void) {
  setHost(NULL);
  setResource(NULL);
  releaseResponse();
  setToken(NULL);
}

void supla_trivial_http::set_string_variable(char **var, int max_len,
                                             const char *src) {
  if (*var) {
    free(*var);
    *var = NULL;
  }

  if (src && strnlen(src, max_len) > 0) {
    *var = strndup(src, max_len);
  }
}

void supla_trivial_http::releaseResponse(void) {
  set_string_variable(&body, 0, NULL);
  set_string_variable(&contentType, 0, NULL);

  this->resultCode = 0;
  this->contentLength = 0;
}

void supla_trivial_http::setHost(const char *host) {
  set_string_variable(&this->host, 1024, host);
}

void supla_trivial_http::setPort(int port) { this->port = port; }

void supla_trivial_http::setResource(const char *resource) {
  set_string_variable(&this->resource, 1024, resource);
}

int supla_trivial_http::getResultCode(void) { return resultCode; }

int supla_trivial_http::getContentLength(void) { return contentLength; }

const char *supla_trivial_http::getContentType(void) { return contentType; }

const char *supla_trivial_http::getBody(void) { return body; }

bool supla_trivial_http::get_addrinfo(void **res) {
  if (!res) {
    return false;
  }

  *res = NULL;

  struct in6_addr serveraddr;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  char port[15];
  snprintf(port, sizeof(port), "%i", this->port);

  char empty[] = "";
  char *server = empty;

  if (host != NULL && strnlen(host, HOST_MAXSIZE) > 0) {
    server = host;
  }

  if (inet_pton(AF_INET, server, &serveraddr) == 1) {
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  } else if (inet_pton(AF_INET6, server, &serveraddr) == 1) {
    hints.ai_family = AF_INET6;
    hints.ai_flags |= AI_NUMERICHOST;
  }

  return getaddrinfo(server, port, &hints, (struct addrinfo **)res) == 0;
}

ssize_t supla_trivial_http::_write(void *ptr, const void *__buf, size_t __n) {
  return write(*(int *)ptr, __buf, __n);
}

ssize_t supla_trivial_http::_read(void *ptr, void *__buf, size_t __n) {
  return read(*(int *)ptr, __buf, __n);
}

bool supla_trivial_http::_should_retry(void *ptr) { return false; }

void supla_trivial_http::write_read(void *ptr, const char *out, char **in) {
  if (_write(ptr, out, strnlen(out, OUTDATA_MAXSIZE)) >= 0) {
    char in_buff[IN_BUFFER_SIZE];
    int size = 0;
    int read_len = 0;

    while ((read_len = _read(ptr, in_buff, sizeof(IN_BUFFER_SIZE))) > 0 ||
           _should_retry(ptr)) {
      if (read_len > 0 && size + read_len < INDATA_MAXSIZE) {
        *in = (char *)realloc((void *)*in, size + read_len + 1);
        memcpy(&((*in)[size]), in_buff, read_len);
        size += read_len;
      }
    }

    if (*in) {
      // size+read_len+1
      (*in)[size] = 0;
    }
  }
}

bool supla_trivial_http::send_recv(const char *out, char **in) {
  struct addrinfo *ai = NULL;
  bool result = false;

  if (!get_addrinfo((void **)&ai)) {
    return false;
  }

  *in = NULL;

  int sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
  if (connect(sfd, ai->ai_addr, (unsigned int)ai->ai_addrlen) == 0) {
    write_read(&sfd, out, in);
  }

  if (sfd != -1) {
    close(sfd);
  }

  if (ai) {
    freeaddrinfo(ai);
    ai = NULL;
  }

  return result;
}

char *supla_trivial_http::header_item_match(const char *item, unsigned int size,
                                            const char *name,
                                            unsigned int name_size) {
  if (size >= name_size && memcmp(item, name, name_size) == 0) {
    char *i = (char *)item;
    return &i[name_size];
  }

  return NULL;
}

void supla_trivial_http::parse_header_item(const char *item, unsigned int size,
                                           bool *chunked) {
  char _http[] = "HTTP/1.1 ";
  char _contentType[] = "Content-Type: ";
  char _contentLength[] = "Content-Length: ";
  char _transferEncoding[] = "Transfer-Encoding: chunked";

  char *match = NULL;
  if (NULL !=
      (match = header_item_match(item, size, _http, sizeof(_http) - 1))) {
    char *space = NULL;
    if ((space = strstr(match, " ")) != NULL) {
      space[0] = 0;
      resultCode = atoi(match);
    }
  } else if (NULL != (match = header_item_match(item, size, _contentType,
                                                sizeof(_contentType) - 1))) {
    if (contentType) {
      free(contentType);
    }
    contentType = strdup(match);

  } else if (chunked &&
             NULL !=
                 (match = header_item_match(item, size, _transferEncoding,
                                            sizeof(_transferEncoding) - 1))) {
    *chunked = true;
  } else if (NULL != (match = header_item_match(item, size, _contentLength,
                                                sizeof(_contentLength) - 1))) {
    contentLength = atoi(match);
  }
}

bool supla_trivial_http::parse(char **in) {
  unsigned int len = strnlen((*in), INDATA_MAXSIZE);
  char next[2] = {'\r', '\n'};
  int pos = 0;

  this->resultCode = 0;
  bool chunked = false;

  for (unsigned int a = 0; a < len - sizeof(next); a++) {
    if (memcmp(&(*in)[a], next, sizeof(next)) == 0) {
      if (a - pos == 0) {
        int n = 0;
        int chunk_pos = 0;
        unsigned long chunk_left = 0;
        bool copy = !chunked;

        for (unsigned int b = a + sizeof(next); b < len; b++) {
          if (chunked) {
            if (chunk_pos == 0) {
              chunk_pos = b;
            }

            if (chunk_left > 0) {
              chunk_left--;
              if (chunk_left == 0) {
                copy = false;
                chunk_pos = b + 2;

                if (b >= len - 1 || (*in)[b] != '\r' || (*in)[b + 1] != '\n') {
                  break;
                }
              }
            }

            if (!copy && b < len - 1 && (*in)[b] == '\r' &&
                (*in)[b + 1] == '\n') {
              (*in)[b] = 0;

              if (b - chunk_pos > 0 && b - chunk_pos <= 8) {
                chunk_left = strtol(&(*in)[chunk_pos], NULL, 16);
                if (chunk_left <= 0) {
                  break;
                }
                copy = true;
              }

              b += 2;

              if (b >= len - 1) {
                break;
              }
            }
          }

          if (copy) {
            (*in)[n] = (*in)[b];
            n++;
          }
        }

        (*in)[n] = 0;
        (*in) = (char *)realloc((char *)(*in), n + 1);

        body = *in;
        *in = NULL;
        break;
      } else {
        (*in)[a] = 0;
        parse_header_item(&(*in)[pos], a - pos, &chunked);
        pos = a + sizeof(next);
      }
    }
  }

  return resultCode > 0;
}

bool supla_trivial_http::request(const char *method, const char *header,
                                 const char *data) {
  int m_len = 0;
  int h_len = 0;
  int r_len = 0;
  bool result = false;

  releaseResponse();

  if (!method || !host || !resource ||
      (m_len = strnlen(method, METHOD_MAXSIZE)) < 3 ||
      (h_len = strnlen(host, HOST_MAXSIZE)) < 1 ||
      (r_len = strnlen(resource, RESOURCE_MAXSIZE) < 1)) {
    return false;
  }

  char *auth = NULL;
  int auth_len = 0;

  if (this->token) {
    auth_len = 30 + strnlen(token, TOKEN_MAXSIZE);
    auth = (char *)malloc(auth_len);

    snprintf(auth, auth_len, "Authorization: Bearer %s\r\n", token);
  }

  size_t size = 100 + m_len + h_len + r_len +
                (header ? strnlen(header, HEADER_MAXSIZE) : 0) +
                (auth ? strnlen(auth, auth_len) : 0);

  if (size > OUTDATA_MAXSIZE) {
    return false;
  }

  char *out_buffer = (char *)malloc(size);
  memset(out_buffer, 0, size);

  snprintf(out_buffer, size - 1,
           "%s %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "User-Agent: supla-server\r\n"
           "%sConnection: close%s%s\r\n\r\n"
           "%s%s",
           method, resource, host, auth ? auth : "", header ? "\r\n" : "",
           header ? header : "", data ? data : "", data ? "\n" : "");

  if (auth) {
    free(auth);
    auth = NULL;
  }

  supla_log(LOG_DEBUG, "%s", out_buffer);

  char *in = NULL;
  send_recv(out_buffer, &in);

  free(out_buffer);
  out_buffer = NULL;

  if (in) {
    result = parse(&in);

    if (in) {
      free(in);
      in = NULL;
    }
  }

  return result;
}

void supla_trivial_http::setToken(char *token) {
  set_string_variable(&this->token, TOKEN_MAXSIZE, token);
}

bool supla_trivial_http::http_get(void) { return request("GET", NULL, NULL); }

bool supla_trivial_http::http_post(void) { return request("POST", NULL, NULL); }
