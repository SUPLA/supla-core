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

#ifndef NOSSL
#include <http/trivialhttps.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "log.h"
#include "tools.h"

typedef struct {
  SSL_CTX *ctx;
  BIO *web;
  SSL *ssl;
  BIO *out;
} _ssl_vars_t;

const char caFile1[] = "/etc/ssl/certs/ca-certificates.crt";
const char caFile2[] = "/etc/pki/tls/certs/ca-bundle.crt";
const char caFile3[] = "/etc/ssl/ca-bundle.pem";
const char caFile4[] = "/etc/pki/tls/cacert.pem";
const char caFile5[] = "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem";

char *supla_trivial_https::caFile = NULL;

// static
void supla_trivial_https::init(void) {
  if (st_file_exists(caFile1)) {
    supla_trivial_https::caFile = (char *)caFile1;
  } else if (st_file_exists(caFile1)) {
    supla_trivial_https::caFile = (char *)caFile2;
  } else if (st_file_exists(caFile1)) {
    supla_trivial_https::caFile = (char *)caFile3;
  } else if (st_file_exists(caFile1)) {
    supla_trivial_https::caFile = (char *)caFile4;
  } else if (st_file_exists(caFile1)) {
    supla_trivial_https::caFile = (char *)caFile5;
  } else {
    supla_trivial_https::caFile = NULL;

    supla_log(LOG_ERR, "Unknown CA certificates file!");
  }
}

supla_trivial_https::supla_trivial_https(const char *host, const char *resource)
    : supla_trivial_http(host, resource) {
  this->ssl_vars = NULL;
  this->port = 443;
}

supla_trivial_https::supla_trivial_https(void) : supla_trivial_http() {
  this->ssl_vars = NULL;
  this->port = 443;
}

supla_trivial_https::~supla_trivial_https(void) { vars_free(); }

void supla_trivial_https::vars_init(void) {
  vars_free();
  _ssl_vars_t *vars = (_ssl_vars_t *)malloc(sizeof(_ssl_vars_t));
  memset(vars, 0, sizeof(_ssl_vars_t));
  ssl_vars = vars;
}

void supla_trivial_https::vars_free(void) {
  if (!ssl_vars) return;
  _ssl_vars_t *vars = (_ssl_vars_t *)ssl_vars;

  if (vars->out) {
    BIO_free(vars->out);
  }

  if (vars->web) {
    BIO_free_all(vars->web);
  }

  if (vars->ctx) {
    SSL_CTX_free(vars->ctx);
  }

  free(ssl_vars);
  ssl_vars = NULL;
}

ssize_t supla_trivial_https::_write(void *ptr, const void *__buf, size_t __n) {
  return BIO_write(((_ssl_vars_t *)ptr)->web, __buf, __n);
}

ssize_t supla_trivial_https::_read(void *ptr, void *__buf, size_t __n) {
  return BIO_read(((_ssl_vars_t *)ptr)->web, __buf, __n);
}

bool supla_trivial_https::_should_retry(void *ptr) {
  return BIO_should_retry(((_ssl_vars_t *)ptr)->web);
}

bool supla_trivial_https::send_recv(const char *out, char **in) {
  if (!host || strnlen(host, HOST_MAXSIZE) <= 0) {
    return false;
  }

  vars_init();
  _ssl_vars_t *vars = (_ssl_vars_t *)ssl_vars;

  const SSL_METHOD *method = SSLv23_method();
  if (method == NULL) {
    supla_log(LOG_DEBUG, "SSLv23 method not exists!");
    vars_free();
    return false;
  }

  vars->ctx = SSL_CTX_new(method);
  if (vars->ctx == NULL) {
    supla_log(LOG_DEBUG, "CTX initialization failed!");
    vars_free();
    return false;
  }

  if (supla_trivial_https::caFile) {
    SSL_CTX_load_verify_locations(vars->ctx, supla_trivial_https::caFile, NULL);
  }

  const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
  SSL_CTX_set_options(vars->ctx, flags);

  vars->web = BIO_new_ssl_connect(vars->ctx);
  if (vars->web == NULL) {
    supla_log(LOG_DEBUG, "Can't create new BIO chain for the ssl connection!");
    vars_free();
    return false;
  }

  int buff_size = (strnlen(host, HOST_MAXSIZE)) + 20;
  char *hostname = (char *)malloc(buff_size);
  snprintf(hostname, buff_size - 1, "%s:%i", host, port);

  long res = BIO_set_conn_hostname(vars->web, hostname);
  free(hostname);

  if (res != 1) {
    supla_log(LOG_DEBUG, "Can't set hostname!");
    vars_free();
    return false;
  }

  BIO_get_ssl(vars->web, &vars->ssl);

  res = SSL_set_tlsext_host_name(vars->ssl, host);

  if (res != 1) {
    supla_log(LOG_DEBUG, "Can't set the server name for ClientHello!");
    vars_free();
    return false;
  }

  vars->out = BIO_new_fp(stdout, BIO_NOCLOSE);

  res = BIO_do_connect(vars->web);

  if (res != 1) {
    supla_log(LOG_DEBUG, "BIO connect failed!");
    vars_free();
    return false;
  }

  res = BIO_do_handshake(vars->web);

  if (res != 1) {
    supla_log(LOG_DEBUG, "BIO handshake failed!");
    vars_free();
    return false;
  }

  X509 *cert = SSL_get_peer_certificate(vars->ssl);
  if (cert) {
    X509_free(cert);
  } else {
    supla_log(LOG_DEBUG, "Can't get server certificate!");
    vars_free();
    return false;
  }

  res = SSL_get_verify_result(vars->ssl);
  if (X509_V_OK != res) {
    supla_log(LOG_DEBUG, "Can't verify server certificate! Code: %i", res);
    vars_free();
    return false;
  }

  write_read(vars, out, in);

  return false;
}

#endif /* NOSSL */
