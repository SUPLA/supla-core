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

#include "jsonconfig/json_config.h"

namespace testing {

ChannelJSONConfigTest::ChannelJSONConfigTest(void) {}
ChannelJSONConfigTest::~ChannelJSONConfigTest(void) {}

TEST_F(ChannelJSONConfigTest, checkDefaults) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  char *str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
  }

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, setUserConfigToNull) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config("{\"c\":1}");

  char *str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  config->set_user_config(NULL);

  str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 10), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, setPropertiesToNull) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_properties("{\"c\":1}");

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  config->set_properties(NULL);

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 10), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, userConfigFormatting) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config("{   }");

  char *str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_user_config("{  \"index\":    0  }");

  str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"index\":0}", 20), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, propertiesFormatting) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_properties("{   }");

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_properties("{  \"index\":    0  }");

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"index\":0}", 20), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, userConfigInvalidFormat) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_user_config("{a=b}");

  char *str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_user_config("{\"x\":\"y\",a=b}");

  str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, propertiesInvalidFormat) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  config->set_properties("{a=b}");

  char *str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_properties("{\"x\":\"y\",a=b}");

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, userConfigRoot) {
  supla_json_config *c1 = new supla_json_config();
  supla_json_config *c2 = new supla_json_config(c1);
  supla_json_config *c3 = new supla_json_config(c2);

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

  char *str = c1->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  c1->set_user_config("{\"c\":1}");

  str = c1->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  c2->set_user_config("{\"c\":2}");

  str = c1->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  c3->set_user_config("{\"c\":3}");

  str = c1->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_user_config();
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

TEST_F(ChannelJSONConfigTest, propertiesRoot) {
  supla_json_config *c1 = new supla_json_config();
  supla_json_config *c2 = new supla_json_config(c1);
  supla_json_config *c3 = new supla_json_config(c2);

  EXPECT_TRUE(c1 != NULL);
  EXPECT_TRUE(c2 != NULL);
  EXPECT_TRUE(c3 != NULL);

  EXPECT_FALSE(c1->is_root_exists());
  EXPECT_TRUE(c2->is_root_exists());
  EXPECT_TRUE(c3->is_root_exists());

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

  char *str = c1->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  c1->set_properties("{\"c\":1}");

  str = c1->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":1}", 10), 0);
    free(str);
    str = NULL;
  }

  c2->set_properties("{\"c\":2}");

  str = c1->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":2}", 10), 0);
    free(str);
    str = NULL;
  }

  c3->set_properties("{\"c\":3}");

  str = c1->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"c\":3}", 10), 0);
    free(str);
    str = NULL;
  }

  str = c3->get_properties();
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

TEST_F(ChannelJSONConfigTest, verificationOfIndependence) {
  supla_json_config *config = new supla_json_config();
  ASSERT_TRUE(config != NULL);

  char *str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_properties("{\"props\":true}");

  str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{}", 5), 0);
    free(str);
    str = NULL;
  }

  config->set_user_config("{\"cfg\":true}");

  str = config->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"cfg\":true}", 20), 0);
    free(str);
    str = NULL;
  }

  str = config->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"props\":true}", 20), 0);
    free(str);
    str = NULL;
  }

  delete config;
}

TEST_F(ChannelJSONConfigTest, copyAndDetach_true) {
  supla_json_config *c1 = new supla_json_config();
  ASSERT_TRUE(c1 != NULL);
  EXPECT_FALSE(c1->is_root_exists());

  c1->set_user_config("{\"u\":123}");
  c1->set_properties("{\"p\":456}");

  supla_json_config *c2 = new supla_json_config(c1, true);
  EXPECT_TRUE(c2 != NULL);

  if (!c2) {
    delete c1;
    return;
  }

  c1->set_user_config("{}");
  c1->set_properties("{}");

  EXPECT_FALSE(c2->is_root_exists());

  char *str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"u\":123}", 20), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"p\":456}", 20), 0);
    free(str);
    str = NULL;
  }

  delete c1;
  delete c2;
}

TEST_F(ChannelJSONConfigTest, copyAndDetach_false) {
  supla_json_config *c1 = new supla_json_config();
  ASSERT_TRUE(c1 != NULL);
  EXPECT_FALSE(c1->is_root_exists());

  c1->set_user_config("{\"u\":123}");
  c1->set_properties("{\"p\":456}");

  supla_json_config *c2 = new supla_json_config(c1, false);
  EXPECT_TRUE(c2 != NULL);

  if (!c2) {
    delete c1;
    return;
  }

  c1->set_user_config("{\"u\":2}");
  c1->set_properties("{\"p\":3}");

  EXPECT_TRUE(c2->is_root_exists());

  char *str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"u\":2}", 20), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"p\":3}", 20), 0);
    free(str);
    str = NULL;
  }

  delete c2;
  delete c1;
}

TEST_F(ChannelJSONConfigTest, assigmentOperator) {
  supla_json_config *c1 = new supla_json_config();
  ASSERT_TRUE(c1 != NULL);
  EXPECT_FALSE(c1->is_root_exists());

  c1->set_user_config("{\"u\":123}");
  c1->set_properties("{\"p\":456}");

  supla_json_config *c2 = new supla_json_config(*c1);
  EXPECT_TRUE(c2 != NULL);

  if (!c2) {
    delete c1;
    return;
  }

  EXPECT_FALSE(c2->is_root_exists());

  char *str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"u\":123}", 20), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"p\":456}", 20), 0);
    free(str);
    str = NULL;
  }

  c1->set_user_config("{\"u\":333}");
  c1->set_properties("{\"p\":444}");

  *c2 = *c1;

  str = c2->get_user_config();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"u\":333}", 20), 0);
    free(str);
    str = NULL;
  }

  str = c2->get_properties();
  EXPECT_TRUE(str != NULL);

  if (str) {
    EXPECT_EQ(strncmp(str, "{\"p\":444}", 20), 0);
    free(str);
    str = NULL;
  }

  delete c2;
  delete c1;
}

} /* namespace testing */
