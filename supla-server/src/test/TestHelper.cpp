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

#include "TestHelper.h"

#include <stdio.h>
#include <string.h>

namespace testing {
// static
void TestHelper::printEscaped(const char *str) {
  size_t size = strlen(str);

  for (size_t a = 0; a < size; a++) {
    if (str[a] == '"' || str[a] == '\\') {
      printf("\\");
    }
    printf("%c", str[a]);
  }
}

// static
unsigned long long TestHelper::usecSince(const struct timeval &tv) {
  struct timeval now = {};
  gettimeofday(&now, NULL);

  return (now.tv_sec * 1000000 + now.tv_usec) -
         (tv.tv_sec * 1000000 + tv.tv_usec);
}

}  // namespace testing
