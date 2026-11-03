/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 */

#ifndef CURL_INSTANCE_TEST_H_
#define CURL_INSTANCE_TEST_H_

#include "gtest/gtest.h"  // NOLINT
#include "http/curl_instance.h"

namespace testing {

class CurlInstanceTest : public Test {};

class TestableCurlInstance : public supla_curl_instance {
 public:
  TestableCurlInstance() : supla_curl_instance(0) {}

  size_t write(void *contents, size_t size, size_t nmemb) {
    return write_callback(contents, size, nmemb, this);
  }

  void set_response_body_size_limit(size_t limit) {
    max_response_body_size = limit;
  }

  size_t response_body_size_limit() const { return max_response_body_size; }
};

}  // namespace testing

#endif  // CURL_INSTANCE_TEST_H_
