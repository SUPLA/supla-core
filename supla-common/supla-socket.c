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

#ifdef _WIN32

#include <WinSock2.h>
#include <assert.h>
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdlib.h>
#include <string.h>

#define MSG_DONTWAIT 0

#define int32_t int
#define uint32_t unsigned int

#define _SERVER_EXCLUDED

#else

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#endif /*ifdef _WIN32*/

#include "lck.h"
#include "log.h"
#include "supla-socket.h"
#include "tools.h"

typedef struct {
  int sfd;

#ifdef _WIN32
  WSADATA wsaData;
#endif /*ifdef _WIN32*/

#ifndef NOSSL
  SSL *ssl;
#endif /*ifndef NOSSL*/
} TSuplaSocket;

typedef struct {
  unsigned char secure;
  int port;
  char *host;

#ifndef NOSSL
  SSL_CTX *ctx;
#endif /*ifndef NOSSL*/

#ifdef __MBED_TLS
  mbedtls_ssl_config mbed_conf;
  mbedtls_x509_crt mbed_cacert;
#endif /*ifdef __MBED_TLS*/

  TSuplaSocket supla_socket;
} TSuplaSocketData;

#ifndef NOSSL
#ifndef _SERVER_EXCLUDED

int ssocket_last_accept_errno = 0;
struct timeval ssocket_last_accept_error_time = {};

void ssocket_ssl_error_log(void) {
  char *errstr;
  int code;
  do {
    code = ERR_get_error();
    if (code) {
      errstr = ERR_error_string(code, NULL);
      supla_log(LOG_ERR, errstr);
    }
  } while (code);
}

SSL_CTX *ssocket_initserverctx(void) {
  SSL_METHOD *method;
  SSL_CTX *ctx;

  OpenSSL_add_all_algorithms();

  method = (SSL_METHOD *)TLS_server_method();
  ctx = SSL_CTX_new(method);

  if (ctx == NULL) ssocket_ssl_error_log();

  return ctx;
}

unsigned char ssocket_loadcertificates(SSL_CTX *ctx, const char *CertFile,
                                       const char *KeyFile) {
  if (!st_file_exists(CertFile)) {
    supla_log(LOG_ERR, "SSL: certificate file doesn't exists");
    return 0;
  }

  if (!st_file_exists(KeyFile)) {
    supla_log(LOG_ERR, "SSL: private key file doesn't exists");
    return 0;
  }

  if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0) {
    ssocket_ssl_error_log();
    return 0;
  }

  if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0) {
    ssocket_ssl_error_log();
    return 0;
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    supla_log(LOG_ERR, "Private key does not match the public certificate");
    return 0;
  }

  X509 *x509 = SSL_CTX_get0_certificate(ctx);
  const ASN1_TIME *notAfter = X509_getm_notAfter(x509);

  int remaining_days = 0, remaining_seconds = 0;
  ASN1_TIME_diff(&remaining_days, &remaining_seconds, NULL, notAfter);
  if (remaining_days <= 60) {
    if (remaining_days > 0 || remaining_seconds > 0) {
      supla_log(LOG_WARNING,
                "The SSL certificate is expiring. Days remaining: %i",
                remaining_days);
    } else {
      supla_log(LOG_ERR, "SSL certificate expired!");
    }
  }

  return 1;
}

int ssocket_get_last_accept_errno(void) { return ssocket_last_accept_errno; }

long int ssocket_get_last_accept_error_time_sec(void) {
  return ssocket_last_accept_error_time.tv_sec;
}

#endif /*ifndef _SERVER_EXCLUDED*/

void ssocket_showcerts(SSL *ssl) {
  X509 *cert;
  char *line;

  cert = SSL_get_peer_certificate(ssl);
  if (cert != NULL) {
    supla_log(LOG_DEBUG, "Server certificates:");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    supla_log(LOG_DEBUG, "Subject: %s", line);
    free(line);
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    supla_log(LOG_DEBUG, "Issuer: %s", line);
    free(line);
    X509_free(cert);
  } else {
    supla_log(LOG_DEBUG, "No certificates.");
  }
}

SSL_CTX *ssocket_client_initctx(void) {
  SSL_METHOD *method;
  SSL_CTX *ctx;

  OpenSSL_add_all_algorithms();
  method = (SSL_METHOD *)TLS_client_method();
  ctx = SSL_CTX_new(method);

  if (ctx == NULL) ssocket_ssl_error_log();

  return ctx;
}
#endif /*ifndef NOSSL*/

#ifndef _SERVER_EXCLUDED
char ssocket_openlistener(void *_ssd) {
  int sd, sflag;
  struct sockaddr_in addr;
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  sd = socket(PF_INET, SOCK_STREAM, 0);

  if (sd == -1) {
    supla_log(LOG_ERR, "Can't create socket");
    return 0;
  }

  sflag = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&sflag, sizeof(sflag));

#ifdef __APPLE__
  setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&sflag, sizeof(sflag));
#endif /*ifdef __APPLE__*/

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(ssd->port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    supla_log(LOG_ERR, "Can't bind port");
    return 0;
  }

  if (listen(sd, 10) != 0) {
    supla_log(LOG_ERR, "Can't configure listening port");
    return 0;
  }

  if (sd != -1) {
    ssd->supla_socket.sfd = sd;
    return 1;
  }

  return 0;
}

// If you want to use SSL support, call SSL_library_init() and
// SSL_load_error_strings() before you use this function with SSL support
void *ssocket_server_init(const char cert[], const char key[], int port,
                          unsigned char secure) {
  TSuplaSocketData *ssd = malloc(sizeof(TSuplaSocketData));

  if (ssd == NULL) return NULL;

  memset(ssd, 0, sizeof(TSuplaSocketData));

  ssd->port = port;
  ssd->supla_socket.sfd = -1;

  if (secure == 1) {
#ifdef NOSSL
    assert(secure == 0);
#else
    ssd->secure = 1;
    ssd->ctx = ssocket_initserverctx();

    if (ssd->ctx == NULL ||
        ssocket_loadcertificates(ssd->ctx, cert, key) == 0) {
      ssocket_free(ssd);
      ssd = NULL;
    }

#endif /*ifdef NOSSL*/
  }

  return ssd;
}

#endif /*ifndef _SERVER_EXCLUDED*/

void ssocket_free(void *_ssd) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  if (ssd) {
    ssocket_close(ssd);

#ifndef NOSSL
#ifndef _SERVER_EXCLUDED

    EVP_cleanup();
    ERR_clear_error();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    ERR_remove_thread_state(NULL);
#pragma GCC diagnostic pop
    ERR_free_strings();
    CRYPTO_cleanup_all_ex_data();

#endif /*ndef _SERVER_EXCLUDED*/
#endif /*ifndef NOSSL*/

    if (ssd->host) {
      free(ssd->host);
      ssd->host = NULL;
    }

    free(ssd);
  }
}

#ifndef _SERVER_EXCLUDED
char ssocket_accept(void *_ssd, unsigned int *ipv4, void **_supla_socket) {
  TSuplaSocket *supla_socket = NULL;
  int client_sd = -1;
  char result = 0;
  struct sockaddr_in addr;
  socklen_t len;
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  *_supla_socket = NULL;

  if (ssd->supla_socket.sfd != -1) {
    len = sizeof(addr);

    client_sd = accept(ssd->supla_socket.sfd, (struct sockaddr *)&addr, &len);

    if (client_sd == -1) {
      if (errno != EAGAIN && errno != ECONNABORTED) {
        ssocket_last_accept_errno = errno;
        gettimeofday(&ssocket_last_accept_error_time, NULL);
      }

      supla_log(LOG_ERR, "Connection accept error %i, %s:%d",
                ssocket_last_accept_errno, inet_ntoa(addr.sin_addr),
                ntohs(addr.sin_port));
    } else {
      supla_log(LOG_INFO, "Connection accepted: %s:%d ClientSD: %i Secure: %i",
                inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), client_sd,
                ssd->secure);

      supla_socket = malloc(sizeof(TSuplaSocket));

      if (supla_socket == NULL) {
        close(client_sd);
        client_sd = -1;
      } else {
        memset(supla_socket, 0, sizeof(TSuplaSocket));
        supla_socket->sfd = -1;

        *ipv4 = htonl(addr.sin_addr.s_addr);

        if (client_sd != -1 && ssd->secure != 1) {
          if (-1 == fcntl(client_sd, F_SETFL, O_NONBLOCK)) {
            supla_log(LOG_ERR, "O_NONBLOCK");

            supla_socket->sfd = client_sd;
            ssocket_supla_socket_close(supla_socket);
            client_sd = -1;
          }
        }

        supla_socket->sfd = client_sd;
        result = 1;
      }
    }
  }

  if (supla_socket && supla_socket->sfd == -1) {
    free(supla_socket);
    supla_socket = NULL;
  }

  *_supla_socket = supla_socket;

  return result;
}

void ssocket_ssl_new(void *_ssd, void *_supla_socket) {
  TSuplaSocket *supla_socket = (TSuplaSocket *)_supla_socket;
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;
  if (supla_socket && supla_socket->sfd != -1 && ssd && ssd->secure == 1) {
    supla_socket->ssl = SSL_new(ssd->ctx);
  }
}

char ssocket_accept_ssl(void *_ssd, void *_supla_socket) {
  TSuplaSocket *supla_socket = (TSuplaSocket *)_supla_socket;
#ifndef NOSSL
  int n;
  struct timeval tv;
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  if (_supla_socket && supla_socket->sfd != -1 && ssd->secure == 1) {
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    n = setsockopt(supla_socket->sfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
                   sizeof(tv));

    if (n == -1) {
      supla_log(LOG_ERR, "SO_RCVTIMEO/%i", tv.tv_sec);
    } else {
      SSL_set_fd(supla_socket->ssl, supla_socket->sfd);
    }

    if (n == -1 || SSL_accept(supla_socket->ssl) < 1) {
      ssocket_supla_socket_close(supla_socket);

      if (n != -1) ssocket_ssl_error_log();

    } else {
      tv.tv_sec = 0;
      tv.tv_usec = 0;

      if (-1 == setsockopt(supla_socket->sfd, SOL_SOCKET, SO_RCVTIMEO,
                           (char *)&tv, sizeof(tv))) {
        supla_log(LOG_ERR, "SO_RCVTIMEO/%i", tv.tv_sec);
        ssocket_supla_socket_close(supla_socket);
      }

      if (-1 == fcntl(supla_socket->sfd, F_SETFL, O_NONBLOCK)) {
        supla_log(LOG_ERR, "O_NONBLOCK");
        ssocket_supla_socket_close(supla_socket);
      }

      supla_log(LOG_INFO, "Cipher: %s, SSL Version: %s, ClientSD: %i",
                SSL_get_cipher(supla_socket->ssl),
                SSL_get_version(supla_socket->ssl), supla_socket->sfd);
    }
  }

#endif /*ifdef NOSSL*/
  return supla_socket->sfd == -1 ? 0 : 1;
}
#endif /*ifndef _SERVER_EXCLUDED*/

void ssocket_supla_socket_close(void *_supla_socket) {
  TSuplaSocket *supla_socket = (TSuplaSocket *)_supla_socket;
  if (supla_socket) {
#ifndef NOSSL

    if (supla_socket->ssl) {
      SSL_shutdown(supla_socket->ssl);
      SSL_free(supla_socket->ssl);
      supla_socket->ssl = NULL;
    }

    ERR_clear_error();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    ERR_remove_thread_state(NULL);
#pragma GCC diagnostic pop

#endif /*ifndef NOSSL */

    if (supla_socket->sfd != -1) {
#ifdef _WIN32
      shutdown(supla_socket->sfd, SD_SEND);
      closesocket(supla_socket->sfd);
      WSACleanup();
#else
      close(supla_socket->sfd);
#endif /*ifdef _WIN32*/

      supla_socket->sfd = -1;
    }
  }
}

void ssocket_supla_socket__close(void *_ssd) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;
  ssocket_supla_socket_close(&ssd->supla_socket);
}

void ssocket_supla_socket_free(void *_supla_socket) {
  if (_supla_socket) {
    ssocket_supla_socket_close(_supla_socket);
    free(_supla_socket);
  }
}

int ssocket_supla_socket_getsfd(void *_supla_socket) {
  return ((TSuplaSocket *)_supla_socket)->sfd;
}

void ssocket_close(void *_ssd) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  if (ssd) {
    ssocket_supla_socket_close(&ssd->supla_socket);

#ifndef NOSSL
    if (ssd->ctx) {
      SSL_CTX_free(ssd->ctx);
      ssd->ctx = NULL;

      EVP_cleanup();
      ERR_free_strings();
    }
#endif /*ifndef NOSSL*/
  }
}

int ssocket_client_openconnection(TSuplaSocketData *ssd, const char *state_file,
                                  int *err) {
  struct in6_addr serveraddr;
  struct addrinfo hints, *res = NULL;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  ssd->supla_socket.sfd = -1;

#ifdef _WIN32
  // struct addrinfo hints, *res;

  if (WSAStartup(MAKEWORD(2, 2), &ssd->supla_socket.wsaData) != 0) {
    return ssd->supla_socket.sfd;
  }
#endif

  char port[15];
  snprintf(port, sizeof(port), "%i", ssd->port);

  char empty[] = "";
  char *server = empty;

  if (ssd->host != NULL && strnlen(ssd->host, 1024) > 0) {
    server = ssd->host;
  }

  if (inet_pton(AF_INET, server, &serveraddr) == 1) {
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_NUMERICHOST;
  } else if (inet_pton(AF_INET6, server, &serveraddr) == 1) {
    hints.ai_family = AF_INET6;
    hints.ai_flags |= AI_NUMERICHOST;
  }

  if (getaddrinfo(server, port, &hints, &res) != 0) {
    if (err) *err = SUPLA_RESULT_HOST_NOT_FOUND;

    supla_write_state_file(state_file, LOG_ERR, "Host not found %s", server);
    return -1;
  }

  ssd->supla_socket.sfd =
      socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (ssd->supla_socket.sfd == -1) {
#ifdef _WIN32
    WSACleanup();
#endif

    return -1;
  }

  if (connect(ssd->supla_socket.sfd, res->ai_addr, res->ai_addrlen) != 0) {
#ifdef _WIN32
    shutdown(ssd->supla_socket.sfd, SD_SEND);
    closesocket(ssd->supla_socket.sfd);
    WSACleanup();
#else
    close(ssd->supla_socket.sfd);
#endif /*ifdef _WIN32*/

    ssd->supla_socket.sfd = -1;

    if (err) *err = SUPLA_RESULT_CANT_CONNECT_TO_HOST;

    supla_write_state_file(state_file, LOG_ERR, "Can't connect to host %s",
                           ssd->host == NULL ? "" : ssd->host);
  }

  freeaddrinfo(res);

#ifdef _WIN32
  if (ssd->secure == 0) {
    u_long iMode = 1;
    ioctlsocket(ssd->supla_socket.sfd, FIONBIO, &iMode);
  }
#endif /*ifdef _WIN32*/

  return ssd->supla_socket.sfd;
}

void *ssocket_client_init(const char host[], int port, unsigned char secure) {
  TSuplaSocketData *ssd = malloc(sizeof(TSuplaSocketData));

  if (ssd == NULL) return NULL;

  supla_log(LOG_INFO, "SSL version: %s", OpenSSL_version(OPENSSL_VERSION));

  memset(ssd, 0, sizeof(TSuplaSocketData));

  ssd->port = port;
  ssd->secure = secure;
  ssd->supla_socket.sfd = -1;

  if (host) {
#ifdef _WIN32
    ssd->host = _strdup(host);
#else
    ssd->host = strdup(host);
#endif
  }

#ifndef NOSSL
  if (secure == 1) {
    SSL_library_init();
    SSL_load_error_strings();

    ssd->ctx = ssocket_client_initctx();

    if (ssd->ctx == NULL) {
      ssocket_free(ssd);
      ssd = NULL;
    }
  }
#endif /*ifndef NOSSL*/

  return ssd;
}

unsigned char ssocket_client_connect(void *_ssd, const char *state_file,
                                     int *err) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;

  ssocket_supla_socket_close(&ssd->supla_socket);

  if (ssocket_client_openconnection(_ssd, state_file, err) == -1) return 0;

  if (ssd->secure == 0) return 1;

#ifndef NOSSL
  ssd->supla_socket.ssl = SSL_new(ssd->ctx);
  SSL_set_fd(ssd->supla_socket.ssl, ssd->supla_socket.sfd);

  if (SSL_connect(ssd->supla_socket.ssl) < 1) {
    ssocket_ssl_error_log();
    ssocket_supla_socket_close(&ssd->supla_socket);

  } else {
#ifdef _WIN32
    u_long iMode = 1;
    ioctlsocket(ssd->supla_socket.sfd, FIONBIO, &iMode);
#else
    fcntl(ssd->supla_socket.sfd, F_SETFL, O_NONBLOCK);
#endif

    supla_log(LOG_DEBUG, "Connected with %s encryption",
              SSL_get_cipher(ssd->supla_socket.ssl));
    SSL_get_cipher(ssd->supla_socket.ssl);
    ssocket_showcerts(ssd->supla_socket.ssl);

    return (1);
  }
#endif /*ifndef NOSSL*/

  return (0);
}

int ssocket_get_fd(void *ssd) {
  return ((TSuplaSocketData *)ssd)->supla_socket.sfd;
}

char ssocket_is_secure(void *_ssd) {
  return ((TSuplaSocketData *)_ssd)->secure == 1;
}

void ssocket_log_ssl_error(void *_supla_socket, int ret) {
  TSuplaSocket *supla_socket = (TSuplaSocket *)_supla_socket;

  // SSL_get_error consumes quite a few CPU cycles.
  int32_t ssl_error = SSL_get_error(supla_socket->ssl, ret);
  if (ssl_error != SSL_ERROR_WANT_READ && ssl_error != SSL_ERROR_WANT_WRITE) {
    switch (ssl_error) {
      case SSL_ERROR_NONE:
        supla_log(LOG_DEBUG, "SSL_ERROR_NONE");
        break;
      case SSL_ERROR_SSL:
        supla_log(LOG_DEBUG, "SSL_ERROR_SSL");
        break;
      case SSL_ERROR_WANT_X509_LOOKUP:
        supla_log(LOG_DEBUG, "SSL_ERROR_WANT_X509_LOOKUP");
        break;
      case SSL_ERROR_SYSCALL:
        supla_log(LOG_DEBUG, "SSL_ERROR_SYSCALL");
        break;
      case SSL_ERROR_ZERO_RETURN:
        supla_log(LOG_DEBUG, "SSL_ERROR_ZERO_RETURN");
        break;
      case SSL_ERROR_WANT_CONNECT:
        supla_log(LOG_DEBUG, "SSL_ERROR_WANT_CONNECT");
        break;
      case SSL_ERROR_WANT_ACCEPT:
        supla_log(LOG_DEBUG, "SSL_ERROR_WANT_ACCEPT");
        break;
    }
  }
}

int ssocket_read(void *_ssd, void *_supla_socket, void *buf, int count) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;
  TSuplaSocket *supla_socket = _supla_socket == NULL
                                   ? &ssd->supla_socket
                                   : (TSuplaSocket *)_supla_socket;

  assert(ssd != NULL);

  if (supla_socket->sfd == -1) {
    return 0;
  }

  if (ssd->secure == 1) {
#ifdef NOSSL
    count = 0;
#else
    count = SSL_read(supla_socket->ssl, buf, count);

    // Only for debug purposes
    // if (count < 0) {
    //  ssocket_log_ssl_error(supla_socket, count);
    //}

#ifndef __SUPLA_SERVER
    if (count < 0) {
      int32_t ssl_error = SSL_get_error(supla_socket->ssl, count);
      if (ssl_error == SSL_ERROR_SYSCALL || ssl_error == SSL_ERROR_SSL) {
        return 0;
      }
    }

#endif

#endif /*ifdef NOSSL*/

  } else {
    count = recv(supla_socket->sfd, buf, count, MSG_DONTWAIT);
  }

#if defined(__DEBUG) && __SSOCKET_WRITE_TO_FILE == 1
  if (count > 0) {
    FILE *f = fopen("ssocket_read.raw", "ab");
    if (f) {
      fwrite(buf, count, 1, f);
      fclose(f);
    }
  }
#endif /*defined(__DEBUG) && __SSOCKET_WRITE_TO_FILE == 1*/

  return count;
}

int ssocket_write(void *_ssd, void *_supla_socket, const void *buf, int count) {
  TSuplaSocketData *ssd = (TSuplaSocketData *)_ssd;
  TSuplaSocket *supla_socket = _supla_socket == NULL
                                   ? &ssd->supla_socket
                                   : (TSuplaSocket *)_supla_socket;

  assert(ssd != NULL);

  if (supla_socket->sfd == -1) {
    return -1;
  }

#if defined(__DEBUG) && __SSOCKET_WRITE_TO_FILE == 1
  if (count > 0) {
    FILE *f = fopen("ssocket_write.raw", "ab");
    if (f) {
      fwrite(buf, count, 1, f);
      fclose(f);
    }
  }
#endif /*defined(__DEBUG) && __SSOCKET_WRITE_TO_FILE == 1*/

  if (ssd->secure == 1) {
#ifndef NOSSL
    count = SSL_write(supla_socket->ssl, buf, count);

    // Only for debug purposes
    // if (count < 0) {
    //  ssocket_log_ssl_error(_supla_socket, count);
    //}
#else
    return -1;
#endif /*ifndef NOSSL*/

  } else {
    count = send(supla_socket->sfd, buf, count,
#ifdef __linux__
                 MSG_NOSIGNAL
#else
                 0
#endif  /*ifdef __linux__*/
    );  // NOLINT
  }

  return count;
}
