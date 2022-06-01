INSERT INTO `supla_user_icons` (`id`, `user_id`, `func`, `image1`, `image2`, `image3`, `image4`) VALUES
(26, 2, 40, 0x89504e470d0a, NULL, NULL, NULL);

UPDATE `supla_scene` SET user_icon_id = 26 WHERE id = 2;
