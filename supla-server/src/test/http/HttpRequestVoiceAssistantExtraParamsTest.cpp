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

#include "HttpRequestVoiceAssistantExtraParamsTest.h"

#include "http/httprequestvoiceassistantextraparams.h"

namespace testing {

HttpRequestVoiceAssistantExtraParamsTest::
    HttpRequestVoiceAssistantExtraParamsTest(void)
    : Test() {}
HttpRequestVoiceAssistantExtraParamsTest::
    ~HttpRequestVoiceAssistantExtraParamsTest(void) {}

TEST_F(HttpRequestVoiceAssistantExtraParamsTest, empty) {
  supla_http_request_voice_assistant_extra_params *params =
      new supla_http_request_voice_assistant_extra_params(NULL, NULL);
  ASSERT_TRUE(params != NULL);
  EXPECT_TRUE(params->getCorrelationTokenPtr() == NULL);
  EXPECT_TRUE(params->getGoogleRequestIdPtr() == NULL);
  delete params;

  params = new supla_http_request_voice_assistant_extra_params();
  EXPECT_TRUE(params != NULL);
  if (params) {
    EXPECT_TRUE(params->getCorrelationTokenPtr() == NULL);
    EXPECT_TRUE(params->getGoogleRequestIdPtr() == NULL);
    delete params;
  }
}

TEST_F(HttpRequestVoiceAssistantExtraParamsTest, keysShouldBeCopied) {
  char correlationToken[] = "ctoken";
  char googleRequestId[] = "greqId";

  supla_http_request_voice_assistant_extra_params *params =
      new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                          googleRequestId);
  ASSERT_TRUE(params != NULL);

  EXPECT_NE((long long)correlationToken,
            (long long)params->getCorrelationTokenPtr());
  EXPECT_NE((long long)googleRequestId,
            (long long)params->getGoogleRequestIdPtr());

  EXPECT_EQ(strncmp(correlationToken, params->getCorrelationTokenPtr(),
                    CORRELATIONTOKEN_MAXSIZE),
            0);

  EXPECT_EQ(strncmp(googleRequestId, params->getGoogleRequestIdPtr(),
                    GOOGLEREQUESTID_MAXSIZE),
            0);
  delete params;
}

TEST_F(HttpRequestVoiceAssistantExtraParamsTest, subChannel) {
  char correlationToken[] = "sdfsdfoppwerjs123pdfksdlkfs::SUB=4";

  supla_http_request_voice_assistant_extra_params *params =
      new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                          NULL);
  ASSERT_TRUE(params != NULL);

  EXPECT_EQ(strncmp("sdfsdfoppwerjs123pdfksdlkfs",
                    params->getCorrelationTokenPtr(), CORRELATIONTOKEN_MAXSIZE),
            0);
  EXPECT_EQ(params->getSubChannel(), 4);
  delete params;
}

TEST_F(HttpRequestVoiceAssistantExtraParamsTest, clone) {
  supla_http_request_voice_assistant_extra_params *params =
      new supla_http_request_voice_assistant_extra_params("abcd", "efgh");
  ASSERT_TRUE(params != NULL);

  supla_http_request_voice_assistant_extra_params *_clone =
      dynamic_cast<supla_http_request_voice_assistant_extra_params *>(
          params->clone());
  EXPECT_TRUE(_clone != NULL);
  if (_clone) {
    EXPECT_EQ(strncmp("abcd", _clone->getCorrelationTokenPtr(),
                      CORRELATIONTOKEN_MAXSIZE),
              0);

    EXPECT_EQ(strncmp("efgh", _clone->getGoogleRequestIdPtr(),
                      GOOGLEREQUESTID_MAXSIZE),
              0);
    delete _clone;
  }

  delete params;
}

} /* namespace testing */
