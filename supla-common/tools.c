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

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "eh.h"
#include "log.h"
#include "tools.h"

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

void st_hook_signals(void) {
  main_thread = pthread_self();

  signal(SIGHUP, st_signal_handler);
  signal(SIGINT, st_signal_handler);
  signal(SIGTERM, st_signal_handler);
  signal(SIGQUIT, st_signal_handler);
  signal(SIGPIPE, SIG_IGN);
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
        srand(tv.tv_usec);
        gettimeofday(&tv, NULL);

        for (a = 0; a < size; a++)
          KEY[a] = (unsigned char)(rand() + tv.tv_usec);  // NOLINT
#else
        unsigned int seed = time(NULL);

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

char *st_get_datetime_str(char buffer[64]) {
  memset(buffer, 0, 64);

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);  // NOLINT
  strftime(buffer, 64, "%c", tm);

  return buffer;
}

int st_hue2rgb(double hue) {
  double r = 0, g = 0, b = 0;

  if (hue >= 360) hue = 0;

  hue /= 60.00;

  long i = (long)hue;
  double f, q, t;
  f = hue - i;

  q = 1.0 - f;
  t = 1.0 - (1.0 - f);

  switch (i) {
    case 0:
      r = 1.00;
      g = t;
      b = 0.00;
      break;

    case 1:
      r = q;
      g = 1.00;
      b = 0.00;
      break;

    case 2:
      r = 0.00;
      g = 1.00;
      b = t;
      break;

    case 3:
      r = 0.00;
      g = q;
      b = 1.00;
      break;

    case 4:
      r = t;
      g = 0.00;
      b = 1.00;
      break;

    default:
      r = 1.00;
      g = 0.00;
      b = q;
      break;
  }

  int rgb = 0;

  rgb |= (unsigned char)(r * 255.00);
  rgb <<= 8;

  rgb |= (unsigned char)(g * 255.00);
  rgb <<= 8;

  rgb |= (unsigned char)(b * 255.00);

  return rgb;
}

void st_random_alpha_string(char *buffer, int buffer_size) {
  int a;

  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char max = sizeof(charset) - 1;
  unsigned int seed = time(NULL);

  buffer[buffer_size - 1] = 0;

  for (a = 0; a < buffer_size - 1; a++) {
    buffer[a] = charset[rand_r(&seed) % max];
  }
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

  unsigned int seed = time(NULL);

  for (a = 0; a < BCRYPT_RABD_SIZE; a++) random[a] = rand_r(&seed) + tv.tv_usec;

  return crypt_gensalt_rn("$2a$", rounds, random, BCRYPT_RABD_SIZE, salt,
                          salt_buffer_size) == NULL
             ? 0
             : 1;
}

char st_bcrypt_hash(char *str, char *salt, char *hash, int hash_buffer_size) {
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

char st_bcrypt_check(char *str, char *hash, int hash_len) {
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

#endif
