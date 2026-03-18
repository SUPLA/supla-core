INSERT INTO supla_value_based_trigger (id, user_id, owning_channel_id, channel_id, channel_group_id, scene_id, schedule_id, push_notification_id, `trigger`, action, action_param, enabled)
VALUES(20, 2, 140, 141, NULL, NULL, NULL, NULL, '{"on_change_to":{"eq":1, "duration_sec": 1}}', 60, NULL, 1),
(21, 2, 141, 158, NULL, NULL, NULL, NULL, '{"on_change_to":{"eq":1, "duration_sec": 1}}', 60, NULL, 1),
(33, 2, 158, 141, NULL, NULL, NULL, NULL, '{"on_change_to":{"ge":50,"duration_sec": 1,"name":"humidity","resume":{"le":10}}}', 60, NULL, 1);
