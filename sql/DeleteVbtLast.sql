DELETE FROM supla_value_based_trigger WHERE id IN (select MAX(id) from supla_value_based_trigger WHERE user_id = 2);
