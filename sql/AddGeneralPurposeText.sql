-- Supla general_purpose_text channel type - MariaDB migration
-- v. >= 29

CREATE TABLE IF NOT EXISTS `supla_gp_text_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `value` varchar(255) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

DROP PROCEDURE IF EXISTS `supla_add_gp_text_log_item`;

DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_gp_text_log_item`(
  IN `_channel_id` INT,
  IN `_value` VARCHAR(255)
)
  NOT DETERMINISTIC
  CONTAINS SQL
  SQL SECURITY DEFINER
INSERT INTO `supla_gp_text_log`(`channel_id`, `date`, `value`)
VALUES (_channel_id, UTC_TIMESTAMP(), _value) ;;
DELIMITER ;
