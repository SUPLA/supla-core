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

#include "sslcrypto.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "lck.h"
#include "log.h"

#ifndef NOSSL

struct CRYPTO_dynlock_value {
  void *lck;
};

static void **ssl_locks = NULL;

static void sslcrypto_locking_function(int mode, int n, const char *file,
                                       int line) {
  if (mode & CRYPTO_LOCK) {
    lck_lock(ssl_locks[n]);
  } else {
    lck_unlock(ssl_locks[n]);
  }
}

static unsigned long sslcrypto_id_function(void) {
  return ((unsigned long)pthread_self());
}

static struct CRYPTO_dynlock_value *sslcrypto_dyn_create_function(
    const char *file, int line) {
  struct CRYPTO_dynlock_value *value;

  value = (struct CRYPTO_dynlock_value *)malloc(
      sizeof(struct CRYPTO_dynlock_value));

  if (!value) {
    goto err;
  }

  value->lck = lck_init();

  return value;

err:
  return (NULL);
}

static void sslcrypto_dyn_lock_function(int mode,
                                        struct CRYPTO_dynlock_value *l,
                                        const char *file, int line) {
  if (mode & CRYPTO_LOCK) {
    lck_lock(l->lck);
  } else {
    lck_unlock(l->lck);
  }
}

static void sslcrypto_dyn_destroy_function(struct CRYPTO_dynlock_value *l,
                                           const char *file, int line) {
  lck_free(l->lck);
  free(l);
}

void sslcrypto_init(void) {
  int i;
  SSL_library_init();
  SSL_load_error_strings();

  supla_log(LOG_INFO, "SSL version: %s", OPENSSL_VERSION_TEXT);

  ssl_locks = malloc(CRYPTO_num_locks() * sizeof(void *));

  if (ssl_locks != 0) {
    for (i = 0; i < CRYPTO_num_locks(); i++) {
      ssl_locks[i] = lck_init();
    }

    // http://openssl.6102.n7.nabble.com/When-to-use-CRYPTO-set-locking-callback-and-CRYPTO-set-id-callback-td7379.html
    CRYPTO_set_locking_callback(sslcrypto_locking_function);
    CRYPTO_set_id_callback(sslcrypto_id_function);
    CRYPTO_set_dynlock_create_callback(sslcrypto_dyn_create_function);
    CRYPTO_set_dynlock_lock_callback(sslcrypto_dyn_lock_function);
    CRYPTO_set_dynlock_destroy_callback(sslcrypto_dyn_destroy_function);
  }
}

void sslcrypto_free(void) {
  int i;
  if (ssl_locks != 0) {
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);

    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_id_callback(NULL);

    for (i = 0; i < CRYPTO_num_locks(); i++) {
      lck_free(ssl_locks[i]);
    }
    free(ssl_locks);
    ssl_locks = NULL;
  }
}

#else
void sslcrypto_init(void) {}
void sslcrypto_free(void) {}
#endif /*NOSSL*/
