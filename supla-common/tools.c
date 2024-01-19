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

const unsigned _supla_int_t st_crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

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
            "Critical Error! Terminate: %i Signal %d:", st_app_terminate, sig);

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

unsigned _supla_int_t st_crc32_checksum(const unsigned char *data,
                                        size_t length) {
  unsigned _supla_int_t crc32 = 0xffffffff;

  while (length--)
    crc32 = (crc32 >> 8) ^ st_crc32_tab[((crc32 ^ *data++) & 0xff)];

  return crc32 ^ 0xffffffff;
}

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
