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

#include "ChannelJSONConfigTest.h"

#include "channeljsonconfig/channeljsonconfig.h"

namespace testing {

ChannelJSONConfigTest::ChannelJSONConfigTest(void) {}
ChannelJSONConfigTest::~ChannelJSONConfigTest(void) {}

TEST_F(ChannelJSONConfigTest, checkDefaults) {
  channel_json_config *config = new channel_json_config();
  ASSERT_TRUE(config != NULL);

  char *str = config->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, formatting) {
  channel_json_config *config = new channel_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_config("{   }");

  char *str = config->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_config("{  \"index\":    0  }");

  str = config->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"index\":0}", 20), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, invalidFormat) {
  channel_json_config *config = new channel_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_config("{a=b}");

  char *str = config->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_config("{\"x\":\"y\",a=b}");

  str = config->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, root) {
  channel_json_config *c1 = new channel_json_config();
  channel_json_config *c2 = new channel_json_config(c1);
  channel_json_config *c3 = new channel_json_config(c2);

  EXPECT_TRUE(c1 != NULL);
  EXPECT_TRUE(c2 != NULL);
  EXPECT_TRUE(c3 != NULL);

  if (!c1 || !c2 || !c3) {
    if (c1) {
      delete c1;
    }

    if (c2) {
      delete c2;
    }

    if (c3) {
      delete c3;
    }
    return;
  }

  char *str = c1->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  c1->set_config("{\"c\":1}");

  str = c1->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  c2->set_config("{\"c\":2}");

  str = c1->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  c3->set_config("{\"c\":3}");

  str = c1->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  delete c1;
  delete c2;
  delete c3;
}

} /* namespace testing */
