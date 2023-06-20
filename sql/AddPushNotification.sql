INSERT INTO `supla_accessid`(`id`, `user_id`, `password`, `caption`, `enabled`, `active_from`, `active_to`, `active_hours`) VALUES (3,2,'abcd','',1,NULL,NULL,NULL);
INSERT INTO `supla_client` (`id`, `access_id`, `guid`, `name`, `reg_ipv4`, `reg_date`, `last_access_ipv4`, `last_access_date`, `software_version`, `protocol_version`, `enabled`, `user_id`, `auth_key`, `caption`, `disable_after_date`) VALUES
(4, 2, 0xa944878fa9141f6241e7c98abaeec331, 'samsung SM-G955N', 3000535933, '2020-03-23 08:03:52', 3000535933, '2020-07-20 05:41:45', 'Android5.1.1/2.3.44', 12, 1, 2, NULL, NULL, NULL),
(11, 2, 0xb79856ba6dc35b49966411d028cb1772, 'motorola Moto G (5S)', 1588637783, '2020-09-01 14:29:43', 3241066828, '2022-03-01 15:20:02', 'Android8.1.0/2.3.77 ', 15, 1, 2, '$2a$04$k0zWbw2Zy5v6CvmViZLDxeOT9dTCotndef9oHlbFyElxV9OKsZoSe', NULL, NULL),
(24, 2, 0xe01e91aa194b3702f6dd3ff373b92d0f, 'iPhone', 3251144141, '2021-01-09 14:06:35', 3251144141, '2021-01-11 17:41:26', 'iOS14.3/2.3.33', 12, 1, 2, '$2a$04$AdUrqNcFCk1aK6BDxbs69O4McV2EEdU7HovT4qdilctF0PjxNl49.', NULL, NULL),
(31, 2, 0x40dcd836bb7dd492cbaf22475328542e, 'iPhone', 785126333, '2021-01-17 18:52:03', 785126333, '2021-01-17 19:03:12', 'iOS14.3/2.3.33', 12, 1, 2, '$2a$04$o5KZTCfT1.lCdyGvYTYsmO1feLwm37efGvSbi2nojul3j/2.hc0Pu', NULL, NULL),
(32, 3, 0xe01e91aaa94b3702f6dd3ff373b92d0f, 'iPhone 2', 3251144141, '2021-01-09 14:06:35', 3251144141, '2021-01-11 17:41:26', 'iOS14.3/2.3.33', 12, 1, 2, '$2a$04$AdAAqNcFCk1aK6BDxbs69O4McV2EEdU7HovT4qdilctF0PjxNl49.', NULL, NULL),
(33, 3, 0xeaae91aaa94b3702f6dd3ff373b92d0f, 'iPhone 3', 3251144141, '2021-01-09 14:06:35', 3251144141, '2021-01-11 17:41:26', 'iOS14.3/2.3.33', 12, 1, 2, '$2a$04$AdUrqNBBcFCk1aK6BDxbs69O4McV2EEdU7HovT4qdilctF0PjxNl49.', NULL, NULL);


INSERT INTO supla_push_notification(id, user_id, channel_id, iodevice_id, managed_by_device, title, body, sound) VALUES(5, 2, 140, 73, 0, 'Abcd', 'Efgh', 11);
INSERT INTO supla_rel_aid_pushnotification(push_notification_id, access_id) VALUES(5, 2);
UPDATE supla_client SET push_token = 'Token 1', platform = 2, devel_env = 0 WHERE id = 4;
UPDATE supla_client SET push_token = 'Token 2', platform = 1, devel_env = 0, app_id = 200 WHERE id = 24;
UPDATE supla_client SET push_token = 'Token 3', platform = 1, devel_env = 1  WHERE id = 31;
UPDATE supla_client SET push_token = 'Token 4', platform = 1, devel_env = 1  WHERE id = 32;
UPDATE supla_client SET push_token = 'Token 5', platform = 1, devel_env = 1  WHERE id = 33;
UPDATE supla_client SET last_access_date = UTC_TIMESTAMP();
