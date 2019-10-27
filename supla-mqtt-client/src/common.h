/*
 * common.h
 *
 *  Created on: 16 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <vector>

bool exists_in_vect(std::vector<std::string>& vect, std::string value);

void replace_string_in_place(std::string& subject, const std::string& search,
                             const std::string& replace);

#endif /* COMMON_H_ */
