UPDATE supla_oauth_access_tokens SET expires_at = UNIX_TIMESTAMP() - 1;
