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

#ifndef ARDUINO

#include "tools.h"

#ifndef __ANDROID_API__
#include <execinfo.h>
#endif /*__ANDROID_API__*/
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "eh.h"
#include "log.h"

#ifdef __OPENSSL_TOOLS
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#endif /*__OPENSSL_TOOLS*/

#ifdef __BCRYPT
#include "crypt_blowfish/ow-crypt.h"
#define BCRYPT_RABD_SIZE 16
#endif

unsigned char st_app_terminate = 0;
pthread_t main_thread;

TEventHandler *st_eh = NULL;

void st_signal_handler(int sig) {
  if (pthread_self() == main_thread) {
    st_app_terminate = 1;
  }

  if (st_eh != 0) eh_raise_event(st_eh);
}

void st_critical_signal_handler(int sig) {
#ifndef __ANDROID_API__
  void *array[20] = {};
  size_t count = 0;

  supla_log(LOG_CRIT,
            "Critical Error! Terminate: %i Signal %d:", st_app_terminate,
            sig);

  count = backtrace(array, 20);
  char **symbols = backtrace_symbols(array, count);

  for (int i = 0; i < count; i++) {
    supla_log(LOG_CRIT, "%s", symbols[i]);
  }

  free(symbols);

  exit(1);
#endif /*__ANDROID_API__*/
}

void st_hook_signals(void) {
  main_thread = pthread_self();

#if defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
  bsd_signal(SIGHUP, st_signal_handler);
  bsd_signal(SIGINT, st_signal_handler);
  bsd_signal(SIGTERM, st_signal_handler);
  bsd_signal(SIGQUIT, st_signal_handler);
  bsd_signal(SIGPIPE, SIG_IGN);
#else
  signal(SIGHUP, st_signal_handler);
  signal(SIGINT, st_signal_handler);
  signal(SIGTERM, st_signal_handler);
  signal(SIGQUIT, st_signal_handler);
  signal(SIGPIPE, SIG_IGN);
#endif /*__ANDROID__*/
}

void st_hook_critical_signals(void) {
#ifndef __ANDROID_API__
  signal(SIGSEGV, st_critical_signal_handler);
  signal(SIGABRT, st_critical_signal_handler);
#endif /*__ANDROID_API__*/
}

unsigned char st_file_exists(const char *fp) {
  unsigned char file_exists = 0;

  if (fp != NULL && strnlen(fp, 1024) > 0) {
    file_exists = access(fp, R_OK) != -1;
    /*
     FILE *F = fopen(fp, "r");

     if ( F ) {
         file_exists = 1;
         fclose(F);
     }
    */
  }

  return file_exists;
}

char st_try_fork(void) {
  pid_t pid, sid;
  pid = fork();
  if (pid < 0) {
    supla_log(LOG_ERR, "Can't fork");
    return 0;
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  sid = setsid();
  if (sid < 0) {
    supla_log(LOG_ERR, "Can't fork");
    return 0;
  }

  if ((chdir("/")) < 0) {
    supla_log(LOG_ERR, "Can't fork");
    return 0;
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  return 1;
}

char st_set_ug_id(int uid, int gid) {
  if (setgid(gid) != 0) {
    supla_log(LOG_ERR, "Can't change group");
    return 0;
  }

  if (setuid(uid) != 0) {
    supla_log(LOG_ERR, "Can't change user");
    return 0;
  }

  return 1;
}

char st_setpidfile(char *pidfile_path) {
  if (pidfile_path != 0 && strnlen(pidfile_path, 1024) > 0) {
    FILE *F = fopen(pidfile_path, "w");
    if (F) {
      char str[32];
      snprintf(str, sizeof(str), "%i\n", getpid());
      fwrite(str, (int)1, strnlen(str, 31), F);
      fclose(F);

      return 1;
    }

    supla_log(LOG_ERR, "Can't create pid file: %s", pidfile_path);
  }

  return 0;
}

void st_delpidfile(char *pidfile_path) {
  if (st_file_exists(pidfile_path) == 1) unlink(pidfile_path);
}

void st_mainloop_init(void) { st_eh = eh_init(); }

void st_mainloop_free(void) { eh_free(st_eh); }

void st_mainloop_wait(int usec) { eh_wait(st_eh, usec); }

char *st_bin2hex(char *buffer, const char *src, size_t len) {
  int a, b;

  if (src == 0 || buffer == 0) return buffer;

  buffer[0] = 0;

  b = 0;

  for (a = 0; a < len; a++) {
    snprintf(&buffer[b], 3, "%02X", (unsigned char)src[a]);  // NOLINT
    b += 2;
  }

  return buffer;
}

void st_guid2hex(char GUIDHEX[SUPLA_GUID_HEXSIZE],
                 const char GUID[SUPLA_GUID_SIZE]) {
  st_bin2hex(GUIDHEX, GUID, SUPLA_GUID_SIZE);
}

void st_authkey2hex(char AuthKeyHEX[SUPLA_AUTHKEY_HEXSIZE],
                    const char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  st_bin2hex(AuthKeyHEX, AuthKey, SUPLA_AUTHKEY_SIZE);
}

char *st_str2hex(char *buffer, const char *str, size_t maxlen) {
  return st_bin2hex(buffer, str, strnlen(str, maxlen));
}

char st_read_randkey_from_file(char *file, char *KEY, int size, char create) {
  FILE *F;
  int a;
  char result = 0;

  if (st_file_exists(file) != 1) {
    if (create == 1 && file != 0 && strnlen(file, 1024) > 0) {
      F = fopen(file, "w");
      if (F) {
        struct timeval tv;
        gettimeofday(&tv, NULL);

#ifdef __ANDROID__
        srand48(tv.tv_usec);
        gettimeofday(&tv, NULL);

        for (a = 0; a < size; a++)
          KEY[a] = (unsigned char)(rand() + tv.tv_usec);  // NOLINT
#else
        unsigned int seed = tv.tv_sec + tv.tv_usec;

        for (a = 0; a < size; a++)
          KEY[a] = (unsigned char)(rand_r(&seed) + tv.tv_usec);
#endif

        if (fwrite(KEY, size, (int)1, F) == 1) {
          result = 1;
        } else {
          supla_log(LOG_ERR, "Can't write to file %s", file);
        }

        fclose(F);
        return result;

      } else {
        supla_log(LOG_ERR, "Can't create file %s", file);
      }
    }

    return 0;
  }

  F = fopen(file, "r");
  if (F) {
    fseek(F, 0, SEEK_END);
    if (ftell(F) == size) {
      fseek(F, 0, SEEK_SET);

      if (fread(KEY, size, (int)1, F) == 1) {
        result = 1;
      } else {
        supla_log(LOG_ERR, "Can't read file %s", file);
      }

    } else {
      supla_log(LOG_ERR, "%s - wrong size", file);
    }

    fclose(F);
  } else {
    supla_log(LOG_ERR, "Can't open file %s", file);
  }

  if (result == 1) {
    result = 0;
    for (a = 0; a < size; a++)
      if ((int)KEY[a] != 0) {
        result = 1;
        break;
      }

    if (result == 0) supla_log(LOG_ERR, "%s - format error", file);
  }

  return result;
}

char st_read_guid_from_file(char *file, char *GUID, char create) {
  return st_read_randkey_from_file(file, GUID, SUPLA_GUID_SIZE, create);
}

char st_read_authkey_from_file(char *file, char *AuthKey, char create) {
  return st_read_randkey_from_file(file, AuthKey, SUPLA_AUTHKEY_SIZE, create);
}

time_t st_get_utc_time(void) {
  time_t now = time(0);
  struct tm *now_tm = gmtime(&now);  // NOLINT
  return mktime(now_tm);
}

char *st_get_zulu_time(char buffer[64]) {
  memset(buffer, 0, 64);

  time_t now = time(0);
  struct tm *tm = gmtime(&now);  // NOLINT
  strftime(buffer, 64, "%Y-%m-%dT%H:%M:%SZ", tm);

  return buffer;
}

char *st_get_datetime_str(char buffer[64]) {
  memset(buffer, 0, 64);

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);  // NOLINT
  strftime(buffer, 64, "%c", tm);

  return buffer;
}

_color_hsv_t st_rgb2hsv(int rgb) {
  _color_hsv_t out;
  double min, max, delta;

  unsigned char r = (unsigned char)((rgb & 0x00FF0000) >> 16);
  unsigned char g = (unsigned char)((rgb & 0x0000FF00) >> 8);
  unsigned char b = (unsigned char)(rgb & 0x000000FF);

  min = r < g ? r : g;
  min = min < b ? min : b;

  max = r > g ? r : g;
  max = max > b ? max : b;

  out.v = max / 255;
  delta = max - min;

  if (delta < 0.00001) {
    out.s = 0;
    out.h = 0;
    return out;
  }
  if (max > 0.0) {
    out.s = (delta / max);
  } else {
    out.s = 0.0;
    out.h = -1;
    return out;
  }
  if (r >= max)
    out.h = (g - b) / delta;
  else if (g >= max)
    out.h = 2.0 + (b - r) / delta;
  else
    out.h = 4.0 + (r - g) / delta;

  out.h *= 60.0;

  if (out.h < 0.0) out.h += 360.0;

  return out;
}

int st_hsv2rgb(_color_hsv_t in) {
  double r = 0, g = 0, b = 0;
  int rgb = 0;

  if (in.s <= 0.0) {
    r = g = b = in.v;
  } else {
    double hh = in.h;

    if (hh >= 360.0) {
      hh = 0.0;
    } else {
      hh /= 60.0;
    }

    _supla_int_t i = (int)hh;
    double f = hh - i;
    double p = in.v * (1.0 - in.s);
    double q = in.v * (1.0 - in.s * f);
    double t = in.v * (1.0 - in.s * (1 - f));

    switch (i) {
      case 0:
        r = in.v;
        g = t;
        b = p;
        break;
      case 1:
        r = q;
        g = in.v;
        b = p;
        break;
      case 2:
        r = p;
        g = in.v;
        b = t;
        break;
      case 3:
        r = p;
        g = q;
        b = in.v;
        break;
      case 4:
        r = t;
        g = p;
        b = in.v;
        break;
      case 5:
        r = in.v;
        g = p;
        b = q;
        break;
      default:
        break;
    }
  }

  rgb = (int)(r * 255.00) & 0xFF;
  rgb <<= 8;
  rgb |= (int)(g * 255.00) & 0xFF;
  rgb <<= 8;
  rgb |= (int)(b * 255.00) & 0xFF;

  return rgb;
}

int st_hue2rgb(double hue) {
  if (hue >= 360) hue = 0;

  _color_hsv_t hsv;
  hsv.h = hue;
  hsv.s = 1.0;
  hsv.v = 1.0;

  return st_hsv2rgb(hsv);
}

void st_random_alpha_string(char *buffer, int buffer_size) {
  int a;

  const char charset[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  char max = sizeof(charset) - 1;

  struct timeval tv;
  gettimeofday(&tv, NULL);

#ifdef __ANDROID__
  srand(tv.tv_usec);
  gettimeofday(&tv, NULL);

  for (a = 0; a < buffer_size - 1; a++) {
    buffer[a] = charset[(rand() + tv.tv_usec) % max];  // NOLINT
  }
#else
  unsigned int seed = tv.tv_sec + tv.tv_usec;
  for (a = 0; a < buffer_size - 1; a++) {
    buffer[a] = charset[rand_r(&seed) % max];
  }
#endif

  buffer[buffer_size - 1] = 0;
}

void st_uuid_v4(char buffer[37]) {
  unsigned char r[16];
  r[0] = 0;
  int a = 0;
  int n = 0;

  struct timeval tv;
  gettimeofday(&tv, NULL);

#ifdef __ANDROID__
  srand(tv.tv_usec);
  gettimeofday(&tv, NULL);

  for (a = 0; a < 16; a++) {
    r[a] = (rand() + tv.tv_usec) % 255;  // NOLINT
  }
#else
  unsigned int seed = tv.tv_sec + tv.tv_usec;
  for (a = 0; a < 16; a++) {
    r[a] = rand_r(&seed) % 255;
  }
#endif

  char hex[3];
  hex[0] = 0;

  for (a = 0; a < 16; a++) {
    snprintf(hex, sizeof(hex), "%02x", r[a]);
    memcpy(&buffer[n], hex, 2);
    n += 2;
    if (n == 8 || n == 13 || n == 18 || n == 23) {
      buffer[n] = '-';
      n++;
    }
  }

  buffer[n] = 0;
}

#ifdef __BCRYPT
char st_bcrypt_gensalt(char *salt, int salt_buffer_size, char rounds) {
  if (salt == NULL || salt_buffer_size == 0) return 0;

  char random[BCRYPT_RABD_SIZE];
  int a;

  if (rounds > 31)
    rounds = 31;
  else if (rounds < 4)
    rounds = 4;

  struct timeval tv;
  gettimeofday(&tv, NULL);

  unsigned int seed = tv.tv_sec + tv.tv_usec;

  for (a = 0; a < BCRYPT_RABD_SIZE; a++) random[a] = rand_r(&seed) + tv.tv_usec;

  return crypt_gensalt_rn("$2a$", rounds, random, BCRYPT_RABD_SIZE, salt,
                          salt_buffer_size) == NULL
             ? 0
             : 1;
}

char st_bcrypt_hash(const char *str, const char *salt, char *hash,
                    int hash_buffer_size) {
  if (str == NULL || hash == NULL || salt == NULL || hash_buffer_size == 0)
    return 0;

  return crypt_rn(str, salt, hash, hash_buffer_size) == NULL ? 0 : 1;
}

char st_bcrypt_crypt(char *str, char *hash, int hash_buffer_size, char rounds) {
  if (str == NULL || hash == NULL || hash_buffer_size == 0) return 0;

  char salt[32];
  if (1 == st_bcrypt_gensalt(salt, 32, rounds)) {
    return st_bcrypt_hash(str, salt, hash, hash_buffer_size);
  }

  return 0;
}

char st_bcrypt_check(const char *str, char *hash, int hash_len) {
  if (str == NULL || hash == NULL || hash_len == 0) return 0;

  char *cmp_hash = malloc(hash_len + 1);
  char result = 0;

  if (st_bcrypt_hash(str, hash, cmp_hash, hash_len + 1) == 1) {
    cmp_hash[hash_len] = 0;

    if (hash_len == strnlen(cmp_hash, hash_len) &&
        memcmp(hash, cmp_hash, hash_len) == 0) {
      result = 1;
    }
  }

  free(cmp_hash);
  return result;
}

char *st_get_authkey_hash_hex(const char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  char AuthKeyHEX[SUPLA_AUTHKEY_HEXSIZE];
  memset(AuthKeyHEX, 0, SUPLA_AUTHKEY_HEXSIZE);

  st_authkey2hex(AuthKeyHEX, AuthKey);

  char hash[BCRYPT_HASH_MAXSIZE];

  if (st_bcrypt_crypt(AuthKeyHEX, hash, BCRYPT_HASH_MAXSIZE, 4)) {
    int size = strnlen(hash, BCRYPT_HASH_MAXSIZE);
    char *hash_hex = (char *)malloc(size * 2 + 1);

    st_str2hex(hash_hex, hash, strnlen(hash, BCRYPT_HASH_MAXSIZE));
    hash_hex[size * 2] = 0;

    return hash_hex;
  }

  return NULL;
}

#endif /* __BCRYPT*/

#ifdef __OPENSSL_TOOLS

char *st_openssl_base64_encode(const char *src, int src_len) {
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;
  char *result = NULL;

  if (src_len <= 0 || src == NULL) {
    return NULL;
  }

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  BIO_write(bio, src, src_len);
  (void)BIO_flush(bio);

  BIO_get_mem_ptr(bio, &bufferPtr);

  result = malloc(bufferPtr->length + 1);
  memcpy(result, bufferPtr->data, bufferPtr->length);
  result[bufferPtr->length] = 0;

  BIO_free_all(bio);

  return result;
}

char *st_openssl_base64_decode(const char *src, int src_len, int *dst_len) {
  BIO *bio, *b64;

  char *buffer = (char *)malloc(src_len + 1);
  memset(buffer, 0, src_len + 1);

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_mem_buf(src, src_len);
  bio = BIO_push(b64, bio);
  BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

  int r = BIO_read(bio, buffer, src_len);
  BIO_free_all(bio);

  if (r < 0) {
    r = 0;
  }

  buffer = realloc(buffer, r + 1);

  if (dst_len) {
    *dst_len = r;
  }

  return buffer;
}

#endif /*__OPENSSL_TOOLS*/
#endif /* !ARDUINO */
