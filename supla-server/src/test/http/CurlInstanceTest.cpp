/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 */

#include "http/CurlInstanceTest.h"

#include <limits>
#include <string>

namespace testing {

using std::string;

TEST_F(CurlInstanceTest, acceptsResponseBodyAtTheLimit) {
  TestableCurlInstance instance;
  string response;
  instance.set_response_body_size_limit(16);
  string body(instance.response_body_size_limit(), 'A');
  instance.set_opt_write_data(&response);

  EXPECT_EQ(instance.write(&body[0], 1, body.size()), body.size());
  EXPECT_EQ(response, body);
}

TEST_F(CurlInstanceTest, rejectsResponseBodyOverTheLimit) {
  TestableCurlInstance instance;
  string response;
  instance.set_response_body_size_limit(16);
  string body(instance.response_body_size_limit() - 1, 'A');
  char extra[] = {'B', 'C'};
  instance.set_opt_write_data(&response);

  ASSERT_EQ(instance.write(&body[0], 1, body.size()), body.size());
  EXPECT_EQ(instance.write(extra, 1, sizeof(extra)), 0);
  EXPECT_EQ(response.size(), body.size());
}

TEST_F(CurlInstanceTest, rejectsOverflowingResponseSize) {
  TestableCurlInstance instance;
  string response;
  char byte = 'A';
  instance.set_opt_write_data(&response);

  EXPECT_EQ(instance.write(&byte, std::numeric_limits<size_t>::max(), 2), 0);
  EXPECT_TRUE(response.empty());
}

}  // namespace testing
