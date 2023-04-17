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

#include "fcm_access_token_provider.h"

#include <assert.h>
#include <openssl/pem.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "log.h"
#include "svrcfg.h"
#include "tools.h"

using std::ifstream;
using std::ostringstream;
using std::string;

supla_fcm_access_token_provider::supla_fcm_access_token_provider(
    supla_abstract_curl_adapter *curl_adapter)
    : supla_abstract_access_token_provider(curl_adapter) {
  private_key = nullptr;
  pk_bio = nullptr;
  init_attempt = false;
}

supla_fcm_access_token_provider::~supla_fcm_access_token_provider(void) {
  if (pk_bio) {
    BIO_free_all(pk_bio);
    pk_bio = nullptr;
  }

  if (private_key) {
    EVP_PKEY_free(private_key);
    private_key = nullptr;
  }
}

void supla_fcm_access_token_provider::initialize(void) {
  if (init_attempt) {
    return;
  }

  init_attempt = true;

  const char *key_file_path = scfg_string(CFG_PUSH_FCM_KEY_FILE);
  if (!key_file_path || key_file_path[0] == 0) {
    return;
  }

  cJSON *json = nullptr;

  {
    std::string key_file_contents;
    std::ifstream key_file(key_file_path);

    if (key_file.is_open()) {
      std::string line;
      while (std::getline(key_file, line)) {
        key_file_contents += line + '\n';
      }
      key_file.close();
    } else {
      supla_log(LOG_ERR, "Can't open the file %s", key_file_path);
    }

    json = cJSON_Parse(key_file_contents.c_str());
  }

  if (!json) {
    supla_log(LOG_ERR, "Can't parse the file %s", key_file_path);
    return;
  }

  string private_key_pem;

  cJSON *pk_json = cJSON_GetObjectItem(json, "private_key");
  if (pk_json && cJSON_IsString(pk_json)) {
    private_key_pem = cJSON_GetStringValue(pk_json);
  }

  cJSON *token_uri_json = cJSON_GetObjectItem(json, "token_uri");
  if (token_uri_json && cJSON_IsString(token_uri_json)) {
    token_uri = cJSON_GetStringValue(token_uri_json);
  }

  cJSON *client_email_json = cJSON_GetObjectItem(json, "client_email");
  if (client_email_json && cJSON_IsString(client_email_json)) {
    client_email = cJSON_GetStringValue(client_email_json);
  }

  cJSON_Delete(json);

  if (private_key_pem.empty()) {
    supla_log(LOG_ERR, "The file %s does not contain a private key.",
              key_file_path);
    return;
  }

  if (token_uri.empty()) {
    supla_log(LOG_ERR, "The %s file does not contain the token URI.",
              key_file_path);
    return;
  }

  if (client_email.empty()) {
    supla_log(LOG_ERR,
              "The %s file does not contain the client's email address.",
              key_file_path);
    return;
  }

  pk_bio = BIO_new_mem_buf((void *)private_key_pem.c_str(), -1);
  assert(pk_bio != nullptr);

  private_key = PEM_read_bio_PrivateKey(pk_bio, nullptr, nullptr, nullptr);
  if (!private_key) {
    BIO_free_all(pk_bio);
    pk_bio = nullptr;

    supla_log(LOG_ERR, "FCM: Error reading private key");
  }
}

string supla_fcm_access_token_provider::base64_url_encoded(char *str, int len) {
  assert(str);

  char *base64 = st_openssl_base64_encode(str, len);
  assert(base64 != nullptr);

  len = strnlen(base64, len * 2);
  for (int a = 0; a < len; a++) {
    switch (base64[a]) {
      case '+':
        base64[a] = '-';
        break;
      case '/':
        base64[a] = '_';
        break;
      case '=':
        base64[a] = 0;
        break;
    }
  }

  string result = base64;
  free(base64);
  return result;
}

string supla_fcm_access_token_provider::base64_url_encoded(cJSON *json) {
  char *str = cJSON_PrintUnformatted(json);
  assert(str != nullptr);
  string result = base64_url_encoded(str, strnlen(str, 4096));
  free(str);
  return result;
}

string supla_fcm_access_token_provider::jwt_sign(void) {
  cJSON *header = cJSON_CreateObject();
  assert(header != nullptr);

  cJSON_AddStringToObject(header, "alg", "RS256");
  cJSON_AddStringToObject(header, "typ", "JWT");

  cJSON *payload = cJSON_CreateObject();
  assert(payload != nullptr);

  struct timeval now = {};
  gettimeofday(&now, nullptr);

  cJSON_AddStringToObject(payload, "aud", token_uri.c_str());
  cJSON_AddStringToObject(payload, "iss", client_email.c_str());
  cJSON_AddNumberToObject(payload, "iat", now.tv_sec);
  cJSON_AddNumberToObject(payload, "exp", now.tv_sec + 60);
  cJSON_AddStringToObject(payload, "email", client_email.c_str());
  cJSON_AddStringToObject(payload, "sub", client_email.c_str());
  cJSON_AddStringToObject(payload, "scope",
                          "https://www.googleapis.com/auth/firebase.messaging");

  string header64 = base64_url_encoded(header);
  string payload64 = base64_url_encoded(payload);

  cJSON_Delete(header);
  cJSON_Delete(payload);

  EVP_MD_CTX *mdctx = EVP_MD_CTX_create();

  char dot = '.';

  EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, private_key);
  EVP_DigestSignUpdate(mdctx, header64.c_str(), header64.size());
  EVP_DigestSignUpdate(mdctx, &dot, 1);
  EVP_DigestSignUpdate(mdctx, payload64.c_str(), payload64.size());
  size_t slen = 0;
  EVP_DigestSignFinal(mdctx, nullptr, &slen);

  unsigned char *signature = new unsigned char[slen];
  EVP_DigestSignFinal(mdctx, signature, &slen);
  EVP_MD_CTX_destroy(mdctx);

  string sig64 = base64_url_encoded((char *)signature, slen);
  delete[] signature;

  string token;

  token.append(header64);
  header64 = "";
  token.append(".");
  token.append(payload64);
  payload64 = "";
  token.append(".");
  token.append(sig64);
  sig64 = "";

  return token;
}

bool supla_fcm_access_token_provider::new_token(
    supla_abstract_curl_adapter *curl_adapter, std::string *token,
    int *expires_in_secs) {
  if (!curl_adapter) {
    return false;
  }

  initialize();

  if (!private_key) {
    return false;
  }

  string jwt = jwt_sign();
  if (jwt.empty()) {
    supla_log(LOG_ERR, "JWT for FCM is empty.");
    return false;
  }

  string post_fields = "grant_type=";
  post_fields.append(
      curl_adapter->escape("urn:ietf:params:oauth:grant-type:jwt-bearer"));
  post_fields.append("&assertion=");
  post_fields.append(jwt);

  jwt.insert(0, "Authorization: Bearer ");

  string request_result;

  curl_adapter->reset();
  curl_adapter->append_header(
      "Content-Type: application/x-www-form-urlencoded");
  curl_adapter->append_header(jwt.c_str());
  curl_adapter->set_opt_url(token_uri.c_str());
  curl_adapter->set_opt_post_fields(post_fields.c_str());
  curl_adapter->set_opt_write_data(&request_result);

  if (!curl_adapter->perform()) {
    // The perform method logs errors by itself
    return false;
  }

  if (curl_adapter->get_response_code() != 200) {
    supla_log(LOG_ERR,
              "Failed to obtain oauth token for FCM authentication. Response "
              "code: %i",
              curl_adapter->get_response_code());
    return false;
  }

  string token_type;

  *expires_in_secs = 0;
  *token = "";

  cJSON *result_json = cJSON_Parse(request_result.c_str());
  if (result_json) {
    cJSON *item = cJSON_GetObjectItem(result_json, "access_token");
    if (item && cJSON_IsString(item)) {
      *token = cJSON_GetStringValue(item);
    }

    item = cJSON_GetObjectItem(result_json, "token_type");
    if (item && cJSON_IsString(item)) {
      token_type = cJSON_GetStringValue(item);
    }

    item = cJSON_GetObjectItem(result_json, "expires_in");
    if (item && cJSON_IsNumber(item)) {
      *expires_in_secs = item->valueint;
    }

    cJSON_Delete(result_json);
  }

  if (token->empty()) {
    supla_log(LOG_ERR,
              "The server response does not contain the oauth token for FCM.");
    return false;
  }

  if (token_type != "Bearer") {
    supla_log(LOG_ERR, "The token type for FCM is different than expected.");
    return false;
  }

  if (*expires_in_secs <= 0) {
    supla_log(LOG_WARNING, "The expiry time of the token is unknown.");
    *expires_in_secs = 1800;
  }

  return true;
}

string supla_fcm_access_token_provider::get_url(void) {
  char *url = scfg_string(CFG_PUSH_FCM_URL);
  return url ? url : "";
}
