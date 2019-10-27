/*
 * common.cpp
 *
 *  Created on: 16 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#include "common.h"

bool exists_in_vect(std::vector<std::string>& vect, std::string value) {
  for (auto str : vect) {
    if (str.compare(value) == 0) return true;
  }
  return false;
}
void replace_string_in_place(std::string& subject, const std::string& search,
                             const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}