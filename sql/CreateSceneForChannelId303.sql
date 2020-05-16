INSERT INTO `supla_scene`(`user_id`, `location_id`, `caption`, `enabled`, `user_icon_id`) VALUES (2,2,'',1,NULL);
SET @id = LAST_INSERT_ID();
INSERT INTO `supla_scene_operation`(`owning_scene_id`, `channel_id`, `channel_group_id`, `scene_id`, `action`, `action_param`, `delay_ms`) VALUES (@id,303,NULL,NULL,0,NULL,0);
