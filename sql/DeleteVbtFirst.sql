DELETE FROM supla_value_based_trigger WHERE id IN (select MIN(id) from supla_value_based_trigger WHERE user_id = 2);
