INSERT INTO `supla_client` (`id`, `access_id`, `guid`, `name`, `reg_ipv4`, `reg_date`, `last_access_ipv4`, `last_access_date`, `software_version`, `protocol_version`, `enabled`, `user_id`, `auth_key`, `caption`, `disable_after_date`) VALUES
(4, 2, 0xa944878fa9141f6241e7c98abaeec331, 'samsung SM-G955N', 3000535933, '2020-03-23 08:03:52', 3000535933, '2020-07-20 05:41:45', 'Android5.1.1/2.3.44', 12, 1, 2, NULL, NULL, NULL);

INSERT INTO supla_push_notification(id, user_id, channel_id, iodevice_id, managed_by_device, title, body, sound) VALUES(5, 2, 140, 73, 0, NULL, NULL, NULL);
INSERT INTO supla_rel_aid_pushnotification(push_notification_id, access_id) VALUES(5, 2);
UPDATE supla_client SET push_token = 'Token 1', platform = 2, devel_env = 0 WHERE id = 4;
