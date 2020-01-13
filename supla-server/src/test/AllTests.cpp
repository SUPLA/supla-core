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

#include "database.h"
#include "gtest/gtest.h"
#include "svrcfg.h"

#ifdef __OPENSSL_TOOLS
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif /*__OPENSSL_TOOLS*/

int main(int argc, char **argv) {
  database::mainthread_init();
  ::testing::InitGoogleTest(&argc, argv);

  if (svrcfg_init(argc, argv) == 0) return EXIT_FAILURE;

  int result = RUN_ALL_TESTS();

  svrcfg_free();

#ifdef __OPENSSL_TOOLS
  EVP_cleanup();
  ERR_clear_error();
  ERR_remove_thread_state(NULL);
  ERR_free_strings();
  CRYPTO_cleanup_all_ex_data();
#endif /*__OPENSSL_TOOLS*/

  database::mainthread_end();

  return result;
}
