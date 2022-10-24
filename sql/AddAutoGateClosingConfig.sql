INSERT INTO `supla_auto_gate_closing` (`channel_id`, `user_id`, `enabled`, `active_from`, `active_to`, `active_hours`, `max_time_open`, `seconds_open`, `closing_attempt`, `last_seen_open`) VALUES
(167, 2, 1, NULL, NULL, NULL, 300, NULL, NULL, NULL),
(174, 2, 1, NULL, NULL, NULL, 400, NULL, NULL, UTC_TIMESTAMP()),
(308, 2, 1, NULL, NULL, NULL, 400, NULL, DATE_ADD(UTC_TIMESTAMP(), INTERVAL -2 SECOND), UTC_TIMESTAMP());
