CREATE EXTENSION timescaledb;
CREATE TABLE supla_em_current_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, phase_no SMALLINT NOT NULL, min NUMERIC(6, 3) NOT NULL, max NUMERIC(6, 3) NOT NULL, avg NUMERIC(6, 3) NOT NULL, PRIMARY KEY(channel_id, date, phase_no));
COMMENT ON COLUMN supla_em_current_log.date IS '(DC2Type:stringdatetime)';
COMMENT ON COLUMN supla_em_current_log.phase_no IS '(DC2Type:tinyint)';
SELECT create_hypertable('supla_em_current_log', 'date');
CREATE TABLE supla_em_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, phase1_fae BIGINT DEFAULT NULL, phase1_rae BIGINT DEFAULT NULL, phase1_fre BIGINT DEFAULT NULL, phase1_rre BIGINT DEFAULT NULL, phase2_fae BIGINT DEFAULT NULL, phase2_rae BIGINT DEFAULT NULL, phase2_fre BIGINT DEFAULT NULL, phase2_rre BIGINT DEFAULT NULL, phase3_fae BIGINT DEFAULT NULL, phase3_rae BIGINT DEFAULT NULL, phase3_fre BIGINT DEFAULT NULL, phase3_rre BIGINT DEFAULT NULL, fae_balanced BIGINT DEFAULT NULL, rae_balanced BIGINT DEFAULT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_em_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_em_log', 'date'); 
CREATE TABLE supla_em_power_active_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, phase_no SMALLINT NOT NULL, min NUMERIC(11, 5) NOT NULL, max NUMERIC(11, 5) NOT NULL, avg NUMERIC(11, 5) NOT NULL, PRIMARY KEY(channel_id, date, phase_no)); 
COMMENT ON COLUMN supla_em_power_active_log.date IS '(DC2Type:stringdatetime)'; 
COMMENT ON COLUMN supla_em_power_active_log.phase_no IS '(DC2Type:tinyint)'; 
SELECT create_hypertable('supla_em_power_active_log', 'date'); 
CREATE TABLE supla_em_voltage_aberration_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, phase_no SMALLINT NOT NULL, count_total INT NOT NULL, count_above INT NOT NULL, count_below INT NOT NULL, sec_above INT NOT NULL, sec_below INT NOT NULL, max_sec_above INT NOT NULL, max_sec_below INT NOT NULL, min_voltage NUMERIC(7, 2) NOT NULL, max_voltage NUMERIC(7, 2) NOT NULL, avg_voltage NUMERIC(7, 2) NOT NULL, measurement_time_sec INT NOT NULL, PRIMARY KEY(channel_id, date, phase_no)); 
COMMENT ON COLUMN supla_em_voltage_aberration_log.date IS '(DC2Type:stringdatetime)'; 
COMMENT ON COLUMN supla_em_voltage_aberration_log.phase_no IS '(DC2Type:tinyint)'; 
SELECT create_hypertable('supla_em_voltage_aberration_log', 'date'); 
CREATE TABLE supla_em_voltage_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, phase_no SMALLINT NOT NULL, min NUMERIC(5, 2) NOT NULL, max NUMERIC(5, 2) NOT NULL, avg NUMERIC(5, 2) NOT NULL, PRIMARY KEY(channel_id, date, phase_no)); 
COMMENT ON COLUMN supla_em_voltage_log.date IS '(DC2Type:stringdatetime)'; 
COMMENT ON COLUMN supla_em_voltage_log.phase_no IS '(DC2Type:tinyint)'; 
SELECT create_hypertable('supla_em_voltage_log', 'date'); 
CREATE TABLE supla_gp_measurement_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, open_value DOUBLE PRECISION NOT NULL, close_value DOUBLE PRECISION NOT NULL, avg_value DOUBLE PRECISION NOT NULL, max_value DOUBLE PRECISION NOT NULL, min_value DOUBLE PRECISION NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_gp_measurement_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_gp_measurement_log', 'date'); 
CREATE TABLE supla_gp_meter_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, value DOUBLE PRECISION NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_gp_meter_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_gp_meter_log', 'date'); 
CREATE TABLE supla_ic_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, counter BIGINT NOT NULL, calculated_value BIGINT NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_ic_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_ic_log', 'date'); 
CREATE TABLE supla_temperature_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, temperature NUMERIC(8, 4) NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_temperature_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_temperature_log', 'date'); 
CREATE TABLE supla_temphumidity_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, temperature NUMERIC(8, 4) NOT NULL, humidity NUMERIC(8, 4) NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_temphumidity_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_temphumidity_log', 'date'); 
CREATE TABLE supla_thermostat_log (channel_id INT NOT NULL, date TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, "on" BOOLEAN NOT NULL, measured_temperature NUMERIC(5, 2) NOT NULL, preset_temperature NUMERIC(5, 2) NOT NULL, PRIMARY KEY(channel_id, date)); 
COMMENT ON COLUMN supla_thermostat_log.date IS '(DC2Type:stringdatetime)'; 
SELECT create_hypertable('supla_thermostat_log', 'date');
CREATE TABLE supla_energy_price_log (date_from TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, date_to TIMESTAMP(0) WITHOUT TIME ZONE NOT NULL, rce NUMERIC(8, 4) DEFAULT NULL, fixing1 NUMERIC(8, 4) DEFAULT NULL, fixing2 NUMERIC(8, 4) DEFAULT NULL, PRIMARY KEY(date_from));
COMMENT ON COLUMN supla_energy_price_log.date_from IS '(DC2Type:stringdatetime)';
COMMENT ON COLUMN supla_energy_price_log.date_to IS '(DC2Type:stringdatetime)';
SELECT create_hypertable('supla_energy_price_log', 'date_from');
ALTER TABLE supla_energy_price_log ADD pdgsz SMALLINT DEFAULT NULL;
COMMENT ON COLUMN supla_energy_price_log.pdgsz IS '(DC2Type:tinyint)';

CREATE OR REPLACE FUNCTION supla_add_em_log_item(
    _channel_id INT,
    _phase1_fae BIGINT,
    _phase1_rae BIGINT,
    _phase1_fre BIGINT,
    _phase1_rre BIGINT,
    _phase2_fae BIGINT,
    _phase2_rae BIGINT,
    _phase2_fre BIGINT,
    _phase2_rre BIGINT,
    _phase3_fae BIGINT,
    _phase3_rae BIGINT,
    _phase3_fre BIGINT,
    _phase3_rre BIGINT,
    _fae_balanced BIGINT,
    _rae_balanced BIGINT
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_em_log (
    channel_id,
    date,
    phase1_fae,
    phase1_rae,
    phase1_fre,
    phase1_rre,
    phase2_fae,
    phase2_rae,
    phase2_fre,
    phase2_rre,
    phase3_fae,
    phase3_rae,
    phase3_fre,
    phase3_rre,
    fae_balanced,
    rae_balanced
)
VALUES (
           _channel_id,
           now() AT TIME ZONE 'UTC',
           _phase1_fae,
           _phase1_rae,
           _phase1_fre,
           _phase1_rre,
           _phase2_fae,
           _phase2_rae,
           _phase2_fre,
           _phase2_rre,
           _phase3_fae,
           _phase3_rae,
           _phase3_fre,
           _phase3_rre,
           _fae_balanced,
           _rae_balanced
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_em_voltage_log_item(
    _date TIMESTAMP,
    _channel_id INTEGER,
    _phase_no SMALLINT,
    _min NUMERIC(5,2),
    _max NUMERIC(5,2),
    _avg NUMERIC(5,2)
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_em_voltage_log (
    date,
    channel_id,
    phase_no,
    min,
    max,
    avg
)
VALUES (
           _date,
           _channel_id,
           _phase_no,
           _min,
           _max,
           _avg
       );
END;
$$;
CREATE OR REPLACE FUNCTION supla_add_em_current_log_item(
    _date timestamp,
    _channel_id integer,
    _phase_no smallint,
    _min numeric(6,3),
    _max numeric(6,3),
    _avg numeric(6,3)
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_em_current_log (
    "date",
    channel_id,
    phase_no,
    min,
    max,
    avg
)
VALUES (
           _date,
           _channel_id,
           _phase_no,
           _min,
           _max,
           _avg
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_gp_measurement_log_item(
    _channel_id integer,
    _open_value double precision,
    _close_value double precision,
    _avg_value double precision,
    _max_value double precision,
    _min_value double precision
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_gp_measurement_log (
    channel_id,
    "date",
    open_value,
    close_value,
    avg_value,
    max_value,
    min_value
)
VALUES (
           _channel_id,
           now() AT TIME ZONE 'UTC',
           _open_value,
           _close_value,
           _avg_value,
           _max_value,
           _min_value
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_gp_meter_log_item(
    _channel_id integer,
    _value double precision
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_gp_meter_log (
    channel_id,
    "date",
    value
)
VALUES (
           _channel_id,
           now() AT TIME ZONE 'UTC',
           _value
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_thermostat_log_item(
    _channel_id integer,
    _measured_temperature numeric(5,2),
    _preset_temperature numeric(5,2),
    _on BOOLEAN
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_thermostat_log (
    channel_id,
    "date",
    measured_temperature,
    preset_temperature,
    "on"
)
VALUES (
           _channel_id,
           now() AT TIME ZONE 'UTC',
           _measured_temperature,
           _preset_temperature,
           _on
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_ic_log_item(
    _channel_id INTEGER,
    _counter BIGINT,
    _calculated_value BIGINT
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_ic_log (
    channel_id,
    date,
    counter,
    calculated_value
)
VALUES (
           _channel_id,
           timezone('UTC', now()),
           _counter,
           _calculated_value
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_em_power_active_log_item(
    _date timestamp,
    _channel_id integer,
    _phase_no smallint,
    _min numeric(11,5),
    _max numeric(11,5),
    _avg numeric(11,5)
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_em_power_active_log (
    "date",
    channel_id,
    phase_no,
    min,
    max,
    avg
)
VALUES (
           _date,
           _channel_id,
           _phase_no,
           _min,
           _max,
           _avg
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_temperature_log_item(
    _channel_id INTEGER,
    _temperature NUMERIC(8,4)
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_temperature_log (
    channel_id,
    date,
    temperature
)
VALUES (
           _channel_id,
           timezone('UTC', now()),
           _temperature
       );
END;
$$;

CREATE OR REPLACE FUNCTION supla_add_temphumidity_log_item(
    _channel_id INTEGER,
    _temperature NUMERIC(8,4),
    _humidity NUMERIC(8,4)
)
RETURNS void
LANGUAGE plpgsql
SET search_path = public
SECURITY DEFINER
AS $$
BEGIN
INSERT INTO supla_temphumidity_log (
    channel_id,
    date,
    temperature,
    humidity
)
VALUES (
           _channel_id,
           timezone('UTC', now()),
           _temperature,
           _humidity
       );
END;
$$;