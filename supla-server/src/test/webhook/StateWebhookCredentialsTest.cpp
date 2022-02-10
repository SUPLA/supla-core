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

#include "StateWebhookCredentialsTest.h"

#include "webhook/statewebhookcredentials.h"

namespace testing {

StateWebhookCredentialsTest::StateWebhookCredentialsTest() {}

TEST_F(StateWebhookCredentialsTest, urlSchemeVerification) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  EXPECT_EQ(c->getScheme(), schemeNotAccepted);
  c->set(NULL, NULL, 0, "http://", NULL);
  EXPECT_EQ(c->getScheme(), schemeHttp);
  c->set(NULL, NULL, 0, "hTtP://", NULL);
  EXPECT_EQ(c->getScheme(), schemeHttp);
  c->set(NULL, NULL, 0, "https:/", NULL);
  EXPECT_EQ(c->getScheme(), schemeNotAccepted);
  c->set(NULL, NULL, 0, "https://", NULL);
  EXPECT_EQ(c->getScheme(), schemeHttps);
  c->set(NULL, NULL, 0, "HTTPS://", NULL);
  EXPECT_EQ(c->getScheme(), schemeHttps);
  c->set(NULL, NULL, 0, "HttPS://", NULL);
  EXPECT_EQ(c->getScheme(), schemeHttps);
  delete c;
}

TEST_F(StateWebhookCredentialsTest, urlValidation) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  EXPECT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://", NULL);
  EXPECT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://", NULL);
  EXPECT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://abc", NULL);
  EXPECT_TRUE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://b.pl", NULL);
  EXPECT_TRUE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://b.pl", NULL);
  EXPECT_TRUE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://b.pl:5000", NULL);
  EXPECT_TRUE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://b.pl:800/abcd.php", NULL);
  EXPECT_TRUE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://b.pl:80a/abcd.php", NULL);
  EXPECT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://b.pl;", NULL);
  EXPECT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://supla.fracz.com/api/state-webhook", NULL);
  EXPECT_TRUE(c->isUrlValid());
  delete c;
}

TEST_F(StateWebhookCredentialsTest, getHost) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  char *host = c->getHost();
  EXPECT_TRUE(host == NULL);
  c->set(NULL, NULL, 0, "http://", NULL);
  host = c->getHost();
  EXPECT_TRUE(host == NULL);
  if (host) {
    free(host);
  }

  c->set(NULL, NULL, 0, "https://localhost", NULL);
  host = c->getHost();
  EXPECT_FALSE(host == NULL);
  if (host) {
    EXPECT_EQ(strcmp(host, "localhost"), 0);
    free(host);
  }

  c->set(NULL, NULL, 0, "https://localhost/", NULL);
  host = c->getHost();
  EXPECT_FALSE(host == NULL);
  if (host) {
    EXPECT_EQ(strcmp(host, "localhost"), 0);
    free(host);
  }

  c->set(NULL, NULL, 0, "https://localhost:", NULL);
  host = c->getHost();
  EXPECT_FALSE(host == NULL);
  if (host) {
    EXPECT_EQ(strcmp(host, "localhost"), 0);
    free(host);
  }

  c->set(NULL, NULL, 0, "https://localhost:8080", NULL);
  host = c->getHost();
  EXPECT_FALSE(host == NULL);
  if (host) {
    EXPECT_EQ(strcmp(host, "localhost"), 0);
    free(host);
  }

  c->set(NULL, NULL, 0, "https://localhost/xyz.php", NULL);
  host = c->getHost();
  EXPECT_FALSE(host == NULL);
  if (host) {
    EXPECT_EQ(strcmp(host, "localhost"), 0);
    free(host);
  }

  delete c;
}

TEST_F(StateWebhookCredentialsTest, getResource) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  char *resource = c->getResource();
  EXPECT_TRUE(resource == NULL);
  c->set(NULL, NULL, 0, "http://", NULL);
  resource = c->getResource();
  EXPECT_TRUE(resource == NULL);
  if (resource) {
    free(resource);
  }

  c->set(NULL, NULL, 0, "https://localhost", NULL);
  resource = c->getResource();
  EXPECT_FALSE(resource == NULL);
  if (resource) {
    EXPECT_EQ(strcmp(resource, "/"), 0);
    free(resource);
  }

  c->set(NULL, NULL, 0, "https://localhost:8080", NULL);
  resource = c->getResource();
  EXPECT_FALSE(resource == NULL);
  if (resource) {
    EXPECT_EQ(strcmp(resource, "/"), 0);
    free(resource);
  }

  c->set(NULL, NULL, 0, "https://localhost:443/", NULL);
  resource = c->getResource();
  EXPECT_FALSE(resource == NULL);
  if (resource) {
    EXPECT_EQ(strcmp(resource, "/"), 0);
    free(resource);
  }

  c->set(NULL, NULL, 0, "https://localhost/xyz.php", NULL);
  resource = c->getResource();
  EXPECT_FALSE(resource == NULL);
  if (resource) {
    EXPECT_EQ(strcmp(resource, "/xyz.php"), 0);
    free(resource);
  }

  c->set(NULL, NULL, 0, "https://localhost/api/xyz.php", NULL);
  resource = c->getResource();
  EXPECT_FALSE(resource == NULL);
  if (resource) {
    EXPECT_EQ(strcmp(resource, "/api/xyz.php"), 0);
    free(resource);
  }

  delete c;
}

TEST_F(StateWebhookCredentialsTest, getPort) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);

  c->set(NULL, NULL, 0, "https://localhost", NULL);
  EXPECT_EQ(c->getPort(), 443);

  c->set(NULL, NULL, 0, "https://localhost/abcd.xml", NULL);
  EXPECT_EQ(c->getPort(), 443);

  c->set(NULL, NULL, 0, "https://localhost:", NULL);
  EXPECT_EQ(c->getPort(), 0);

  c->set(NULL, NULL, 0, "https://localhost:8080", NULL);
  EXPECT_EQ(c->getPort(), 8080);

  c->set(NULL, NULL, 0, "https://localhost:8081/", NULL);
  EXPECT_EQ(c->getPort(), 8081);

  delete c;
}

} /* namespace testing */
