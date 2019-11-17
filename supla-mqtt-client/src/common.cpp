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

#include "common.h"

bool exists_in_vect(std::vector<std::string>* vect, const std::string& value) {
  for (auto str : *vect) {
    if (str.compare(value) == 0) return true;
  }
  return false;
}
void replace_string_in_place(std::string* subject, const std::string& search,
                             const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject->find(search, pos)) != std::string::npos) {
    subject->replace(pos, search.length(), replace);
    pos += replace.length();
  }
}
