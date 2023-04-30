INSERT INTO supla_push_notification(id, user_id, channel_id, iodevice_id, managed_by_device, title, body, sound) VALUES(5, 2, 140, 73, 0, 'Abcd', 'Efgh', 11);
INSERT INTO supla_rel_aid_pushnotification(push_notification_id, access_id) VALUES(5, 2);
UPDATE supla_client SET push_token = 'Token 1', platform = 2, devel_env = 0 WHERE id = 4;
UPDATE supla_client SET push_token = 'Token 2', platform = 1, devel_env = 0 WHERE id = 24;
UPDATE supla_client SET push_token = 'Token 3', platform = 1, devel_env = 1  WHERE id = 31;
