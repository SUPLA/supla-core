-- Supla general_purpose_text channel type - TimescaleDB (TSDB) migration
-- v. >= 29

CREATE TABLE IF NOT EXISTS supla_gp_text_log (
  channel_id INT NOT NULL,
  date TIMESTAMPTZ(0) NOT NULL,
  value VARCHAR(255) NOT NULL,
  PRIMARY KEY(channel_id, date)
);
COMMENT ON COLUMN supla_gp_text_log.date IS '(DC2Type:stringdatetime)';
SELECT create_hypertable('supla_gp_text_log', 'date', if_not_exists => TRUE);
