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

#include "client/client_dao.h"

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "proto.h"
#include "svrcfg.h"
#include "tools.h"

supla_client_dao::supla_client_dao(void)
    : supla_abstract_client_dao(), svrdb() {}

supla_client_dao::~supla_client_dao() {}

int supla_client_dao::oauth_add_client_id(void) {
  char random_id[51];
  char secret[51];

  st_random_alpha_string(random_id, 51);
  st_random_alpha_string(secret, 51);

  for (int a = 0; a < 51; a++) {
    random_id[a] = tolower(random_id[a]);
    secret[a] = tolower(secret[a]);
  }

  char sql[] = "CALL `supla_oauth_add_client_for_app`(?,?,@id)";

  MYSQL_BIND pbind[2];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_STRING;
  pbind[0].buffer = (char *)random_id;
  pbind[0].buffer_length = strnlen(random_id, 50);

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)secret;
  pbind[1].buffer_length = strnlen(secret, 50);

  return add_by_proc_call(sql, pbind, 2);
}

int supla_client_dao::oauth_get_client_id(bool create) {
  int id = 0;
  MYSQL_STMT *stmt = NULL;

  if (!stmt_get_int(
          (void **)&stmt, &id, NULL, NULL, NULL,
          "SELECT `id` FROM `supla_oauth_clients` WHERE `type` = 2 LIMIT 1",
          NULL, 0)) {
    id = 0;
  }

  if (id == 0 && create) {
    id = oauth_add_client_id();
  }

  return id;
}

bool supla_client_dao::oauth_get_token(TSC_OAuthToken *token, int user_id,
                                       int access_id,
                                       bool *storage_connect_error) {
  // TODO(anyone): This code has been ported from database.cpp. The business
  // logic code should be moved outside of the DAO.

  if (token == nullptr) {
    return false;
  }

  memset(token, 0, sizeof(TSC_OAuthToken));

  if (oauth_get_client_id(true) == 0 || svrcfg_oauth_url_base64 == NULL ||
      svrcfg_oauth_url_base64_len <= 0 ||
      svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2 >
          SUPLA_OAUTH_TOKEN_MAXSIZE) {
    return false;
  }

  if (!connect()) {
    if (storage_connect_error) {
      *storage_connect_error = true;
    }
    return false;
  }

  st_random_alpha_string(token->Token, CFG_OAUTH_TOKEN_SIZE + 1);
  token->Token[CFG_OAUTH_TOKEN_SIZE] = '.';

  memcpy(&token->Token[CFG_OAUTH_TOKEN_SIZE + 1], svrcfg_oauth_url_base64,
         svrcfg_oauth_url_base64_len);
  token->Token[svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 1] = 0;
  token->TokenSize = svrcfg_oauth_url_base64_len + CFG_OAUTH_TOKEN_SIZE + 2;
  token->ExpiresIn = (unsigned)scfg_int(CFG_OAUTH_TOKEN_LIFETIME);

  int ExpiresIn = token->ExpiresIn + (unsigned)time(NULL);

  char sql[] = "CALL `supla_oauth_add_token_for_app`(?,?,?,?,@id)";

  MYSQL_BIND pbind[4];
  memset(pbind, 0, sizeof(pbind));

  pbind[0].buffer_type = MYSQL_TYPE_LONG;
  pbind[0].buffer = (char *)&user_id;

  pbind[1].buffer_type = MYSQL_TYPE_STRING;
  pbind[1].buffer = (char *)token->Token;
  pbind[1].buffer_length = token->TokenSize - 1;

  pbind[2].buffer_type = MYSQL_TYPE_LONG;
  pbind[2].buffer = (char *)&ExpiresIn;

  pbind[3].buffer_type = MYSQL_TYPE_LONG;
  pbind[3].buffer = (char *)&access_id;

  bool result = add_by_proc_call(sql, pbind, 4) > 0;

  disconnect();

  return result;
}

bool supla_client_dao::set_reg_enabled(int user_id, int device_reg_time_sec,
                                       int client_reg_time_sec) {
  if (!connect()) {
    return false;
  }

  char sql[100] = {};
  snprintf(sql, sizeof(sql),
           "CALL `supla_set_registration_enabled`(%i, %i, %i)", user_id,
           device_reg_time_sec, client_reg_time_sec);

  bool result = query(sql, true) == 0;

  disconnect();

  return result;
}
