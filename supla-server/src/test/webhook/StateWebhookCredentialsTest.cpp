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

TEST_F(StateWebhookCredentialsTest, urlProtocolVerification) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  ASSERT_FALSE(c->isUrlProtocolAccepted());
  c->set(NULL, NULL, 0, "http://", NULL);
  ASSERT_FALSE(c->isUrlProtocolAccepted());
  c->set(NULL, NULL, 0, "https:/", NULL);
  ASSERT_FALSE(c->isUrlProtocolAccepted());
  c->set(NULL, NULL, 0, "https://", NULL);
  ASSERT_TRUE(c->isUrlProtocolAccepted());
  c->set(NULL, NULL, 0, "HTTPS://", NULL);
  ASSERT_TRUE(c->isUrlProtocolAccepted());
  c->set(NULL, NULL, 0, "HttPS://", NULL);
  ASSERT_TRUE(c->isUrlProtocolAccepted());
  delete c;
}

TEST_F(StateWebhookCredentialsTest, urlValidation) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  ASSERT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "http://", NULL);
  ASSERT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://abc", NULL);
  ASSERT_FALSE(c->isUrlValid());
  c->set(NULL, NULL, 0, "https://b.pl", NULL);
  ASSERT_TRUE(c->isUrlValid());

  delete c;
}

TEST_F(StateWebhookCredentialsTest, getHost) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  char *host = c->getHost();
  ASSERT_TRUE(host == NULL);
  c->set(NULL, NULL, 0, "http://", NULL);
  host = c->getHost();
  ASSERT_TRUE(host == NULL);

  c->set(NULL, NULL, 0, "https://localhost", NULL);
  host = c->getHost();
  ASSERT_FALSE(host == NULL);
  ASSERT_TRUE(strcmp(host, "localhost") == 0);
  free(host);

  c->set(NULL, NULL, 0, "https://localhost/xyz.php", NULL);
  host = c->getHost();
  ASSERT_FALSE(host == NULL);
  ASSERT_TRUE(strcmp(host, "localhost") == 0);
  free(host);

  delete c;
}

TEST_F(StateWebhookCredentialsTest, getResource) {
  supla_state_webhook_credentials *c =
      new supla_state_webhook_credentials(NULL);
  c->set(NULL, NULL, 0, NULL, NULL);
  char *resource = c->getResource();
  ASSERT_TRUE(resource == NULL);
  c->set(NULL, NULL, 0, "http://", NULL);
  resource = c->getResource();
  ASSERT_TRUE(resource == NULL);

  c->set(NULL, NULL, 0, "https://localhost", NULL);
  resource = c->getResource();
  ASSERT_FALSE(resource == NULL);
  ASSERT_TRUE(strcmp(resource, "/") == 0);
  free(resource);

  c->set(NULL, NULL, 0, "https://localhost/xyz.php", NULL);
  resource = c->getResource();
  ASSERT_FALSE(resource == NULL);
  ASSERT_TRUE(strcmp(resource, "/xyz.php") == 0);
  free(resource);

  c->set(NULL, NULL, 0, "https://localhost/api/xyz.php", NULL);
  resource = c->getResource();
  ASSERT_FALSE(resource == NULL);
  ASSERT_TRUE(strcmp(resource, "/api/xyz.php") == 0);
  free(resource);

  delete c;
}

} /* namespace testing */
