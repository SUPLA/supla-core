INSERT INTO supla_push_notification(id, user_id, channel_id, iodevice_id, managed_by_device, title, body, sound)
VALUES(500, 2, 140, 73, 0, 'Abcd', 'Efgh', 11);

INSERT INTO supla_value_based_trigger (id, user_id, owning_channel_id, channel_id, channel_group_id, scene_id, schedule_id, push_notification_id, `trigger`, action, action_param, enabled)
VALUES(20, 2, 140, 141, NULL, NULL, NULL, NULL, '{"on_change_to":{"eq":1}}', 60, NULL, 1),
(21, 2, 140, 173, NULL, NULL, NULL, NULL, '{"on_change_to":{"eq":1}}', 51, '{"percentage":45}', 1),
(22, 2, 140, NULL, 1, NULL, NULL, NULL, '{"on_change_to":{"eq":1}}', 70, NULL, 1),
(30, 2, 141, NULL, NULL, 6, NULL, NULL, '{"on_change_to":{"eq":0}}', 3000, NULL, 1),
(31, 2, 141, NULL, NULL, NULL, 1, NULL, '{"on_change_to":{"eq":1}}', 210, NULL, 1),
(32, 2, 141, NULL, NULL, NULL, NULL, 500, '{"on_change_to":{"eq":1}}', 220, NULL, 1)