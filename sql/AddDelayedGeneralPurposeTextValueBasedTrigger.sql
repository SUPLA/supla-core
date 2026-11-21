INSERT INTO supla_dev_channel (
	id,
	iodevice_id,
	user_id,
	channel_number,
	caption,
	type,
	func,
	flist,
	param1,
	param2,
	param3,
	text_param1,
	text_param2,
	text_param3,
	alt_icon,
	hidden,
	location_id,
	flags,
	user_icon_id,
	user_config,
	param4,
	properties,
	sub_device_id,
	conflict_details
)
VALUES (
	320,
	83,
	2,
	100,
	'GPT delayed',
	9020,
	540,
	0,
	0,
	0,
	0,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	NULL,
	0,
	NULL,
	'[]',
	0,
	NULL,
	0,
	NULL
);

INSERT INTO supla_push_notification(id, user_id, channel_id, iodevice_id, managed_by_device, title, body, sound)
VALUES(500, 2, 320, 83, 0, 'Abcd', 'Efgh', 11);

INSERT INTO supla_value_based_trigger (id, user_id, owning_channel_id, channel_id, channel_group_id, scene_id, schedule_id, push_notification_id, `trigger`, action, action_param, enabled)
VALUES(34, 2, 320, NULL, NULL, NULL, NULL, 500, '{"on_change_to":{"eq":"match","duration_sec":1}}', 220, NULL, 1);