-- Test-only SQL extracted from Cloud Version20261011120000.php.
-- Source SHA256: 0d4a89ce4fb87c2439560224d31f0f749ccf280b8cd364bd58d0ceeaf55191fb
-- Cloud owns production migration; Core only runs this in disposable test DB.
CREATE TABLE IF NOT EXISTS supla_resource_access_pair (
            id INT AUTO_INCREMENT NOT NULL, user_id INT NOT NULL,
            destination_device_id INT NOT NULL, source_channel_id INT NOT NULL, source_device_id INT NOT NULL,
            PRIMARY KEY(id), UNIQUE KEY resource_access_identity(user_id,destination_device_id,source_channel_id),
            KEY IDX_RESOURCE_ACCESS_USER(user_id),
            CONSTRAINT resource_access_user FOREIGN KEY(user_id) REFERENCES supla_user(id) ON DELETE CASCADE,
            CONSTRAINT resource_access_distinct CHECK(source_device_id<>destination_device_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
CREATE TABLE IF NOT EXISTS supla_resource_access_decision (
            id INT AUTO_INCREMENT NOT NULL, pair_id INT NOT NULL, open_pair_id INT DEFAULT NULL,
            status VARCHAR(8) NOT NULL, permissions INT NOT NULL, revision INT NOT NULL DEFAULT 1,
            PRIMARY KEY(id), UNIQUE KEY resource_access_open(open_pair_id), KEY IDX_RESOURCE_ACCESS_PAIR(pair_id),
            CONSTRAINT resource_access_pair FOREIGN KEY(pair_id) REFERENCES supla_resource_access_pair(id) ON DELETE CASCADE,
            CONSTRAINT resource_access_mask CHECK(permissions BETWEEN 1 AND 7),
            CONSTRAINT resource_access_revision CHECK(revision>0),
            CONSTRAINT resource_access_status CHECK(status IN ('PENDING','ACCEPTED','REJECTED','REMOVED')),
            CONSTRAINT resource_access_open_state CHECK(
                (status IN ('PENDING','REJECTED') AND open_pair_id IS NOT NULL AND open_pair_id=pair_id)
                OR (status IN ('ACCEPTED','REMOVED') AND open_pair_id IS NULL))
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
ALTER TABLE supla_resource_access_decision
            ADD COLUMN IF NOT EXISTS requested_at DATETIME DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
            ADD COLUMN IF NOT EXISTS last_decision_at DATETIME DEFAULT NULL COMMENT '(DC2Type:utcdatetime)';
DROP TRIGGER IF EXISTS resource_access_decision_creation;
DELIMITER $$
CREATE TRIGGER resource_access_decision_creation BEFORE INSERT ON supla_resource_access_decision
            FOR EACH ROW BEGIN
                SET NEW.requested_at=UTC_TIMESTAMP();
                SET NEW.last_decision_at=IF(NEW.status IN ('ACCEPTED','REJECTED','REMOVED'),NEW.requested_at,NULL);
            END$$
DELIMITER ;
DROP TRIGGER IF EXISTS resource_access_decision_revision;
DELIMITER $$
CREATE TRIGGER resource_access_decision_revision BEFORE UPDATE ON supla_resource_access_decision
            FOR EACH ROW BEGIN
                SET NEW.requested_at=OLD.requested_at;
                SET NEW.last_decision_at=IF(OLD.status<>NEW.status,UTC_TIMESTAMP(),OLD.last_decision_at);
                IF NEW.pair_id<>OLD.pair_id OR NEW.id<>OLD.id THEN
                    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Access decision identity is immutable';
                END IF;
                IF OLD.status<>'PENDING' AND NEW.permissions<>OLD.permissions THEN
                    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Only pending access may change permissions';
                END IF;
                IF OLD.status='REMOVED' OR
                    (OLD.status='ACCEPTED' AND NEW.status NOT IN ('ACCEPTED','REMOVED')) OR
                    (OLD.status='REJECTED' AND NEW.status NOT IN ('REJECTED','ACCEPTED','REMOVED')) THEN
                    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid access decision transition';
                END IF;
                IF OLD.status<>NEW.status OR OLD.permissions<>NEW.permissions THEN
                    IF OLD.revision>=2147483647 THEN
                        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Access decision revision exhausted';
                    END IF;
                    SET NEW.revision=OLD.revision+1;
                ELSE
                    SET NEW.revision=OLD.revision;
                END IF;
            END$$
DELIMITER ;
