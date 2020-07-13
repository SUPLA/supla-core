INSERT INTO `supla_dev_channel_group`(`user_id`, `caption`, `func`, `hidden`, `location_id`, `alt_icon`, `user_icon_id`) VALUES (2,'',110,0,2,NULL,NULL);
SET @id = LAST_INSERT_ID();

INSERT INTO `supla_rel_cg`(`channel_id`, `group_id`) VALUES (173,@id);
INSERT INTO `supla_rel_cg`(`channel_id`, `group_id`) VALUES (303,@id);
