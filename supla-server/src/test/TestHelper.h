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

#ifndef TESTHELPER_H_
#define TESTHELPER_H_

#include <stddef.h>
#include <sys/time.h>

namespace testing {
class TestHelper {
 public:
  static void printEscaped(const char *str);
  static unsigned long long usecSince(const struct timeval &tv);
  static unsigned long long timeDiffUs(const struct timeval &now,
                                       const struct timeval &then);
  static void randomize(char *data, size_t size);
};
}  // namespace testing

#endif /*TESTHELPER_H_*/
