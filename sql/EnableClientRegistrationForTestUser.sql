UPDATE `supla_user` SET client_reg_enabled = NOW() + INTERVAL 1 DAY WHERE email = 'test@supla.org';
