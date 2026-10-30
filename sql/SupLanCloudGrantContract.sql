-- Test fixture only. Never deploy these procedure definitions from Core.
-- Production ownership: supla-cloud Version20261016120000.php, cloud-grants/.
-- Snapshot: supla-cloud 2d747a49dbf402c2e6253aff8dd55dae17f4f483 (2026-10-18).
-- Content: schema.sql, valid.sql and all supla_*.sql from that migration.
-- This pinned test contract is maintained explicitly with SQL API changes;
-- standard Core tests require no sibling checkout or fixture generator.

CREATE TABLE supla_resource_access_grant (
 user_id INT NOT NULL, destination_device_id INT NOT NULL, source_device_id INT NOT NULL,
 binding_target_channel_id INT DEFAULT NULL,
 resource_type TINYINT UNSIGNED NOT NULL DEFAULT 1, resource_id INT NOT NULL,
 origin_type SMALLINT UNSIGNED NOT NULL, origin_id BIGINT UNSIGNED NOT NULL,
 permissions TINYINT UNSIGNED NOT NULL, source_type INT NOT NULL, source_function INT NOT NULL,
 peer_eligible TINYINT UNSIGNED NOT NULL, requires_peer TINYINT UNSIGNED NOT NULL DEFAULT 0,
 PRIMARY KEY(user_id,destination_device_id,resource_type,resource_id,origin_type,origin_id),
 KEY resource_grant_origin(user_id,origin_type,origin_id),
 KEY resource_grant_source(user_id,source_device_id,destination_device_id),
 KEY resource_grant_channel(resource_id),
 CONSTRAINT logical_grant_binding FOREIGN KEY(binding_target_channel_id) REFERENCES supla_dev_channel(id) ON DELETE CASCADE,
 CONSTRAINT logical_grant_user FOREIGN KEY(user_id) REFERENCES supla_user(id) ON DELETE CASCADE,
 CONSTRAINT logical_grant_destination FOREIGN KEY(destination_device_id) REFERENCES supla_iodevice(id) ON DELETE CASCADE,
 CONSTRAINT logical_grant_source FOREIGN KEY(source_device_id) REFERENCES supla_iodevice(id) ON DELETE CASCADE,
 CONSTRAINT logical_grant_channel FOREIGN KEY(resource_id) REFERENCES supla_dev_channel(id) ON DELETE CASCADE,
 CONSTRAINT logical_grant_binding_identity CHECK(
  (origin_type=2 AND binding_target_channel_id IS NULL)
  OR (origin_type=1 AND (origin_id & 255) IN (5,6) AND binding_target_channel_id IS NOT NULL AND resource_id=(origin_id >> 8))
  OR (origin_type=1 AND (origin_id & 255) NOT IN (5,6) AND binding_target_channel_id IS NULL)),
 CONSTRAINT logical_grant_valid CHECK(resource_type=1 AND origin_type IN (1,2) AND origin_id>0
   AND permissions BETWEEN 1 AND 7 AND peer_eligible IN (0,1) AND requires_peer IN (0,1)
   AND source_device_id<>destination_device_id AND (requires_peer=0 OR peer_eligible=1))
) ENGINE=InnoDB;
CREATE TABLE supla_suplan_changed_pair (
 association_id BIGINT UNSIGNED NOT NULL, user_id INT NOT NULL,
 revision BIGINT UNSIGNED NOT NULL DEFAULT 1,
 PRIMARY KEY(association_id), KEY suplan_dirty_user(user_id,association_id),
 CONSTRAINT suplan_dirty_pair FOREIGN KEY(association_id) REFERENCES supla_suplan_peer_association(id) ON DELETE CASCADE,
 CONSTRAINT suplan_dirty_user FOREIGN KEY(user_id) REFERENCES supla_user(id) ON DELETE CASCADE
) ENGINE=InnoDB;
CREATE TABLE supla_resource_access_origin_lock (
 user_id INT NOT NULL, origin_type SMALLINT UNSIGNED NOT NULL, origin_id BIGINT UNSIGNED NOT NULL,
 PRIMARY KEY(user_id,origin_type,origin_id),
 CONSTRAINT resource_origin_user FOREIGN KEY(user_id) REFERENCES supla_user(id) ON DELETE CASCADE
) ENGINE=InnoDB;

DELIMITER $$
CREATE OR REPLACE VIEW supla_resource_access_grant_valid AS
 SELECT g.* FROM supla_resource_access_grant g
 JOIN supla_dev_channel c ON c.id=g.resource_id AND c.user_id=g.user_id AND c.iodevice_id=g.source_device_id
  AND c.type=g.source_type AND c.func=g.source_function AND c.func<>0
 JOIN supla_iodevice s ON s.id=g.source_device_id AND s.user_id=g.user_id
 JOIN supla_iodevice d ON d.id=g.destination_device_id AND d.user_id=g.user_id
 LEFT JOIN supla_dev_channel b ON b.id=g.binding_target_channel_id AND b.user_id=g.user_id AND b.iodevice_id=g.destination_device_id
 WHERE (g.binding_target_channel_id IS NULL OR (b.type=2900 AND b.func=IF((g.origin_id & 255)=5,960,970)))
  AND d.is_virtual=0 AND d.protocol_version>=29 AND (d.flags & 524288)<>0
  AND (g.requires_peer=0 OR (s.is_virtual=0 AND c.is_virtual=0 AND s.protocol_version>=29 AND (s.flags & 524288)<>0))
  AND ((g.permissions & 4)=0 OR COALESCE(JSON_LENGTH(JSON_EXTRACT(c.properties,'$.actionTriggerCapabilities')),0)>0)$$
CREATE OR REPLACE PROCEDURE supla_resource_access_pending(IN p_user INT,IN p_destination INT,IN p_channel INT,
 IN p_permissions INT,IN p_mode INT,IN p_allow BOOLEAN)
SQL SECURITY DEFINER
proc: BEGIN
 DECLARE v_lock INT; DECLARE v_pair INT; DECLARE v_source INT; DECLARE v_missing INT;
 DECLARE v_decision INT DEFAULT NULL; DECLARE v_status VARCHAR(8); DECLARE v_permissions INT; DECLARE v_pending INT;
 IF @@in_transaction=0 OR p_permissions NOT BETWEEN 1 AND 7 OR p_mode NOT IN (1,2) OR (p_mode=2 AND p_permissions<>1) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid SupLAN demand'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT c.iodevice_id INTO v_source FROM supla_dev_channel c JOIN supla_iodevice d ON d.id=p_destination AND d.user_id=p_user
  JOIN supla_suplan_device_state i ON i.device_id=d.id AND i.user_id=p_user
  WHERE c.id=p_channel AND c.user_id=p_user AND c.func<>0 AND c.iodevice_id<>d.id
   AND d.is_virtual=0 AND d.protocol_version>=29 AND (d.flags & 524288)<>0;
 IF v_source IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid SupLAN demand owner'; END IF;
 IF p_mode=1 AND NOT EXISTS(SELECT 1 FROM supla_iodevice s JOIN supla_suplan_device_state i ON i.device_id=s.id AND i.user_id=s.user_id
  WHERE s.id=v_source AND s.user_id=p_user AND s.is_virtual=0 AND s.protocol_version>=29 AND (s.flags & 524288)<>0) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Source does not support SupLAN peer'; END IF;
 -- This procedure is called ONLY for an actual ENSURE, never value publication/quiet checks.
 -- Recording existing consent's requested delivery is independent of permission to create Pending.
 IF p_mode=2 THEN
  UPDATE supla_resource_access_decision d JOIN supla_resource_access_pair p ON p.id=d.pair_id
   SET d.requested_delivery_mode=2
   WHERE p.user_id=p_user AND p.destination_device_id=p_destination AND p.source_channel_id=p_channel
    AND p.source_device_id=v_source AND d.status='ACCEPTED' AND (d.permissions & 3)<>0
    AND d.requested_delivery_mode<>2;
 END IF;
 IF p_allow IS NULL OR p_allow<>1 THEN LEAVE proc; END IF;
 SELECT COALESCE(BIT_OR(permissions),0) INTO v_permissions FROM supla_resource_access_grant_valid
  WHERE user_id=p_user AND destination_device_id=p_destination AND resource_type=1 AND resource_id=p_channel;
 SET v_missing=(IF((p_permissions & 2)<>0,p_permissions & 6,p_permissions) & ~v_permissions);
 IF v_missing=0 THEN LEAVE proc; END IF;
 INSERT INTO supla_resource_access_pair(user_id,destination_device_id,source_device_id,source_channel_id)
  VALUES(p_user,p_destination,v_source,p_channel) ON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id);
 SET v_pair=LAST_INSERT_ID();
 SELECT id,status,permissions INTO v_decision,v_status,v_permissions FROM supla_resource_access_decision
  WHERE open_pair_id=v_pair FOR UPDATE;
 IF v_status='REJECTED' THEN LEAVE proc; END IF;
 IF v_decision IS NULL THEN
  INSERT INTO supla_resource_access_decision(pair_id,open_pair_id,status,permissions,requested_delivery_mode)
   VALUES(v_pair,v_pair,'PENDING',v_missing,p_mode);
 ELSE
  SET v_pending=(v_permissions | v_missing);
  SET v_pending=IF((v_pending & 2)<>0,v_pending & 6,v_pending);
  UPDATE supla_resource_access_decision SET permissions=v_pending,
   requested_delivery_mode=IF(v_pending=1,p_mode,1) WHERE id=v_decision;
 END IF;
END$$
CREATE OR REPLACE PROCEDURE supla_resource_access_replace(IN p_user INT, IN p_type INT, IN p_origin BIGINT UNSIGNED, IN p_grants LONGTEXT)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_done BOOLEAN DEFAULT FALSE; DECLARE v_source INT; DECLARE v_destination INT; DECLARE v_create BOOLEAN;
 DECLARE v_lock INT;
 DECLARE affected CURSOR FOR
  SELECT source_device_id,destination_device_id,MAX(requires_peer) FROM (
   SELECT source_device_id,destination_device_id,requires_peer FROM supla_resource_access_grant
    WHERE user_id=p_user AND origin_type=p_type AND origin_id=p_origin
   UNION ALL SELECT source_device_id,destination_device_id,requires_peer FROM JSON_TABLE(p_grants,'$[*]' COLUMNS(
    source_device_id INT PATH '$.source',destination_device_id INT PATH '$.destination',requires_peer INT PATH '$.requiresPeer')) j
  ) pairs GROUP BY source_device_id,destination_device_id ORDER BY source_device_id,destination_device_id;
 DECLARE CONTINUE HANDLER FOR NOT FOUND SET v_done=TRUE;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Logical replacement requires transaction'; END IF;
 IF p_type NOT IN (1,2) OR p_origin=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid logical origin identity'; END IF;
 IF JSON_VALID(p_grants)=0 OR JSON_TYPE(p_grants)<>'ARRAY' THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid logical grant JSON'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 IF v_lock IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Unknown SupLAN account'; END IF;
 INSERT IGNORE INTO supla_resource_access_origin_lock VALUES(p_user,p_type,p_origin);
 SELECT user_id INTO v_lock FROM supla_resource_access_origin_lock WHERE user_id=p_user AND origin_type=p_type AND origin_id=p_origin FOR UPDATE;
 IF EXISTS(SELECT 1 FROM JSON_TABLE(p_grants,'$[*]' COLUMNS(
   source_device_id INT PATH '$.source',destination_device_id INT PATH '$.destination',resource_id INT PATH '$.resource',
   permissions INT PATH '$.permissions',source_type INT PATH '$.type',source_function INT PATH '$.function',
   binding_target_channel_id INT PATH '$.bindingTarget',peer_eligible INT PATH '$.peerEligible',requires_peer INT PATH '$.requiresPeer')) j
  LEFT JOIN supla_dev_channel c ON c.id=j.resource_id AND c.user_id=p_user AND c.iodevice_id=j.source_device_id
  LEFT JOIN supla_dev_channel b ON b.id=j.binding_target_channel_id AND b.user_id=p_user AND b.iodevice_id=j.destination_device_id
  LEFT JOIN supla_iodevice s ON s.id=j.source_device_id AND s.user_id=p_user
  LEFT JOIN supla_iodevice d ON d.id=j.destination_device_id AND d.user_id=p_user
  WHERE c.id IS NULL OR s.id IS NULL OR d.id IS NULL OR c.func=0 OR c.func<>j.source_function OR c.type<>j.source_type
   OR j.permissions NOT BETWEEN 1 AND 7 OR j.permissions IS NULL OR j.source_device_id=j.destination_device_id
   OR (j.binding_target_channel_id IS NOT NULL AND (b.id IS NULL OR p_type<>1 OR (p_origin & 255) NOT IN (5,6)
    OR b.type<>2900 OR b.func<>IF((p_origin & 255)=5,960,970)))
   OR (p_type=2 AND NOT EXISTS(SELECT 1 FROM supla_resource_access_decision decision
    JOIN supla_resource_access_pair pair_identity ON pair_identity.id=decision.pair_id
    WHERE decision.id=p_origin AND decision.status='ACCEPTED' AND pair_identity.user_id=p_user
     AND pair_identity.destination_device_id=j.destination_device_id AND pair_identity.source_device_id=j.source_device_id
     AND pair_identity.source_channel_id=j.resource_id
     AND IF((decision.permissions & 2)<>0,decision.permissions | 1,decision.permissions)=IF((j.permissions & 2)<>0,j.permissions | 1,j.permissions)))
   OR (p_type=1 AND (NOT EXISTS(SELECT 1 FROM supla_dev_channel consumer WHERE consumer.id=(p_origin >> 8) AND consumer.user_id=p_user)
    OR NOT ((p_origin & 255) BETWEEN 1 AND 6 OR (p_origin & 255) BETWEEN 32 AND 41 OR (p_origin & 255) BETWEEN 64 AND 83)))
   OR j.peer_eligible IS NULL OR j.requires_peer IS NULL
   OR (j.peer_eligible=1 AND ((s.flags & 524288)=0 OR (d.flags & 524288)=0 OR s.is_virtual<>0 OR d.is_virtual<>0
    OR c.is_virtual<>0 OR s.protocol_version<29 OR d.protocol_version<29))) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid logical resource authority'; END IF;
 -- Capture the old and new affected pair set before removing this origin.
 OPEN affected;
 BEGIN
  DECLARE v_pairs LONGTEXT DEFAULT '[]'; DECLARE v_index INT DEFAULT 0;
  read_pairs: LOOP
   FETCH affected INTO v_source,v_destination,v_create;
   IF v_done THEN LEAVE read_pairs; END IF;
   SET v_pairs=JSON_ARRAY_APPEND(v_pairs,'$',JSON_OBJECT('s',v_source,'d',v_destination,'c',v_create));
  END LOOP;
  CLOSE affected;
  DELETE FROM supla_resource_access_grant WHERE user_id=p_user AND origin_type=p_type AND origin_id=p_origin;
  INSERT INTO supla_resource_access_grant(user_id,source_device_id,destination_device_id,resource_id,origin_type,origin_id,
   permissions,source_type,source_function,binding_target_channel_id,peer_eligible,requires_peer)
   SELECT p_user,source_device_id,destination_device_id,resource_id,p_type,p_origin,
    IF((permissions & 2)<>0,permissions | 1,permissions),source_type,source_function,binding_target_channel_id,peer_eligible,requires_peer
   FROM JSON_TABLE(p_grants,'$[*]' COLUMNS(source_device_id INT PATH '$.source',destination_device_id INT PATH '$.destination',
    resource_id INT PATH '$.resource',permissions INT PATH '$.permissions',source_type INT PATH '$.type',source_function INT PATH '$.function',
    binding_target_channel_id INT PATH '$.bindingTarget',peer_eligible INT PATH '$.peerEligible',requires_peer INT PATH '$.requiresPeer')) j;
  WHILE v_index<JSON_LENGTH(v_pairs) DO
   CALL supla_suplan_project(p_user,JSON_VALUE(v_pairs,CONCAT('$[',v_index,'].s')),
    JSON_VALUE(v_pairs,CONCAT('$[',v_index,'].d')),JSON_VALUE(v_pairs,CONCAT('$[',v_index,'].c')));
   SET v_index=v_index+1;
  END WHILE;
 END;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_accept_identity(IN p_user INT,IN p_device INT,IN p_root BIGINT UNSIGNED,IN p_channels VARBINARY(512))
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT; DECLARE v_index INT DEFAULT 1; DECLARE v_previous BIGINT UNSIGNED DEFAULT 0; DECLARE v_channel BIGINT UNSIGNED;
 IF @@in_transaction=0 OR p_root NOT BETWEEN 1 AND 4294967294 OR LENGTH(p_channels)%4<>0 OR p_channels IS NULL THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid accepted SERVER identity'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SET v_lock=NULL;
 SELECT id INTO v_lock FROM supla_iodevice WHERE id=p_device AND user_id=p_user AND is_virtual=0
  AND protocol_version>=29 AND (flags & 524288)<>0 FOR UPDATE;
 IF v_lock IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid SERVER device'; END IF;
 WHILE v_index<=LENGTH(p_channels) DO
  SET v_channel=CONV(HEX(SUBSTRING(p_channels,v_index,4)),16,10);
  IF v_channel<=v_previous OR v_channel>2147483647 OR NOT EXISTS(
   SELECT 1 FROM supla_dev_channel WHERE id=v_channel AND user_id=p_user AND iodevice_id=p_device) THEN
   SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid accepted channel mapping'; END IF;
  SET v_previous=v_channel; SET v_index=v_index+4;
 END WHILE;
 INSERT INTO supla_suplan_device_state(device_id,user_id,current_root_epoch,accepted_channels)
  VALUES(p_device,p_user,p_root,p_channels) ON DUPLICATE KEY UPDATE current_root_epoch=VALUES(current_root_epoch),accepted_channels=VALUES(accepted_channels);
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_changed_ack(IN p_user INT,IN p_id BIGINT UNSIGNED,IN p_revision BIGINT UNSIGNED)
SQL SECURITY DEFINER
BEGIN
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 DELETE FROM supla_suplan_changed_pair WHERE user_id=p_user AND association_id=p_id AND revision=p_revision;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_cleanup(IN p_user INT,IN p_device INT)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_done BOOLEAN DEFAULT FALSE; DECLARE v_id BIGINT UNSIGNED; DECLARE v_source INT; DECLARE v_destination INT; DECLARE v_lock INT;
 DECLARE pairs CURSOR FOR SELECT id,source_device_id,destination_device_id FROM supla_suplan_peer_association
  WHERE user_id=p_user AND (source_device_id=p_device OR destination_device_id=p_device) ORDER BY source_device_id,destination_device_id;
 DECLARE CONTINUE HANDLER FOR NOT FOUND SET v_done=TRUE;
 IF @@in_transaction=0 OR EXISTS(SELECT 1 FROM supla_iodevice WHERE id=p_device AND user_id=p_user) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Cleanup requires deleted or foreign device'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 OPEN pairs;
 pair_loop: LOOP
  FETCH pairs INTO v_id,v_source,v_destination;
  IF v_done THEN LEAVE pair_loop; END IF;
  UPDATE supla_suplan_peer_association SET source_removed=IF(v_source=p_device,1,source_removed),
   destination_removed=IF(v_destination=p_device,1,destination_removed) WHERE id=v_id;
  CALL supla_suplan_project(p_user,v_source,v_destination,FALSE);
  -- A deleted endpoint cannot acknowledge. Never wait for its former connection.
  UPDATE supla_suplan_peer_association SET source_empty_acked=IF(v_source=p_device,1,source_empty_acked),
   destination_empty_acked=IF(v_destination=p_device,1,destination_empty_acked) WHERE id=v_id;
  -- No recorded Source context means no Destination key could ever have been forwarded.
  UPDATE supla_suplan_peer_association SET destination_empty_acked=1
   WHERE id=v_id AND source_removed=1 AND provisioned_root_epoch=0 AND provisioned_peer_generation=0;
  DELETE FROM supla_suplan_peer_association WHERE id=v_id AND source_empty_acked=1 AND destination_empty_acked=1;
 END LOOP;
 CLOSE pairs;
 DELETE FROM supla_suplan_device_state WHERE device_id=p_device AND user_id=p_user;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_observe_root(IN p_user INT,IN p_id BIGINT UNSIGNED)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 UPDATE supla_suplan_peer_association a LEFT JOIN supla_suplan_device_state s ON s.device_id=a.source_device_id AND s.user_id=a.user_id
  SET a.source_empty_acked=1 WHERE a.user_id=p_user AND a.id=p_id AND a.lifecycle=2 AND a.source_empty_acked=0
  AND (a.source_removed=1 OR (a.provisioned_root_epoch<>0 AND s.current_root_epoch<>a.provisioned_root_epoch));
  DELETE FROM supla_suplan_peer_association WHERE user_id=p_user AND id=p_id
   AND source_empty_acked=1 AND destination_empty_acked=1 AND (source_removed=1 OR destination_removed=1);
  UPDATE supla_suplan_peer_association SET lifecycle=3 WHERE user_id=p_user AND id=p_id
   AND source_empty_acked=1 AND destination_empty_acked=1;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_peer_ack(IN p_user INT,IN p_id BIGINT UNSIGNED,IN p_root BIGINT UNSIGNED,
 IN p_generation BIGINT UNSIGNED,IN p_revision BIGINT UNSIGNED,IN p_side INT,IN p_empty BOOLEAN)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT; DECLARE v_id BIGINT UNSIGNED DEFAULT NULL; DECLARE v_lifecycle INT;
 IF @@in_transaction=0 OR p_side NOT IN (1,2) OR p_empty NOT IN (0,1) OR p_root NOT BETWEEN 1 AND 4294967294 THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Invalid SupLAN ACK'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT a.id,a.lifecycle INTO v_id,v_lifecycle FROM supla_suplan_peer_association a
  LEFT JOIN supla_suplan_device_state s ON s.device_id=a.source_device_id AND s.user_id=a.user_id
  WHERE a.id=p_id AND a.user_id=p_user AND a.peer_generation=p_generation AND a.acl_revision=p_revision
  AND ((p_empty=1 AND LENGTH(a.canonical_acl)=0 AND a.lifecycle IN (2,3) AND a.provisioned_root_epoch=p_root)
   OR (p_empty=0 AND LENGTH(a.canonical_acl)>0 AND a.lifecycle=1 AND a.source_removed=0 AND a.destination_removed=0
    AND s.current_root_epoch=p_root AND (p_side=1 OR (a.provisioned_root_epoch=p_root AND a.provisioned_peer_generation=p_generation)))) FOR UPDATE;
 IF v_id IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Stale SupLAN ACK'; END IF;
 IF p_empty THEN
  UPDATE supla_suplan_peer_association SET source_empty_acked=IF(p_side=1,1,source_empty_acked),
   destination_empty_acked=IF(p_side=2,1,destination_empty_acked) WHERE id=v_id;
  DELETE FROM supla_suplan_peer_association WHERE user_id=p_user AND id=v_id
   AND source_empty_acked=1 AND destination_empty_acked=1 AND (source_removed=1 OR destination_removed=1);
  UPDATE supla_suplan_peer_association SET lifecycle=3 WHERE user_id=p_user AND id=v_id
   AND source_empty_acked=1 AND destination_empty_acked=1;
 ELSEIF p_side=1 THEN
  -- Commit EXACT acknowledged Source context, including the first root=0 -> accepted root.
  -- Never reset or advance generation/revision after Source has confirmed those bytes.
  UPDATE supla_suplan_peer_association SET provisioned_root_epoch=p_root,provisioned_peer_generation=p_generation WHERE id=v_id;
 END IF;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_peer_demand(IN p_user INT,IN p_destination INT,IN p_channel INT)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_source INT DEFAULT NULL; DECLARE v_lock INT; DECLARE v_channels VARBINARY(512); DECLARE v_offset INT DEFAULT 1; DECLARE v_found BOOLEAN DEFAULT FALSE;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT g.source_device_id,i.accepted_channels INTO v_source,v_channels FROM supla_resource_access_grant g
  JOIN supla_suplan_device_state i ON i.device_id=g.source_device_id AND i.user_id=g.user_id
  JOIN supla_suplan_device_state d ON d.device_id=g.destination_device_id AND d.user_id=g.user_id
  WHERE g.user_id=p_user AND g.destination_device_id=p_destination AND g.resource_type=1 AND g.resource_id=p_channel
   AND g.peer_eligible=1 LIMIT 1;
 WHILE v_offset<=LENGTH(v_channels) DO
  IF CONV(HEX(SUBSTRING(v_channels,v_offset,4)),16,10)=p_channel THEN SET v_found=TRUE; END IF;
  SET v_offset=v_offset+4;
 END WHILE;
 IF v_source IS NULL OR v_found=FALSE THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='No committed SupLAN authority'; END IF;
 CALL supla_suplan_project(p_user,v_source,p_destination,TRUE);
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_peer_distrust(IN p_user INT,IN p_id BIGINT UNSIGNED,
 IN p_generation BIGINT UNSIGNED,IN p_revision BIGINT UNSIGNED)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT; DECLARE v_id BIGINT UNSIGNED DEFAULT NULL;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT id INTO v_id FROM supla_suplan_peer_association WHERE user_id=p_user AND id=p_id
  AND peer_generation=p_generation AND acl_revision=p_revision AND peer_generation<4294967295 FOR UPDATE;
 IF v_id IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Stale or exhausted SupLAN distrust'; END IF;
 UPDATE supla_suplan_peer_association SET peer_generation=peer_generation+1,acl_revision=1,
  source_empty_acked=0,destination_empty_acked=0,change_revision=change_revision+1 WHERE id=v_id;
 INSERT INTO supla_suplan_changed_pair(association_id,user_id,revision)
  SELECT id,user_id,change_revision FROM supla_suplan_peer_association WHERE id=v_id ON DUPLICATE KEY UPDATE revision=VALUES(revision);
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_peer_prepare(IN p_user INT,IN p_id BIGINT UNSIGNED,
 IN p_generation BIGINT UNSIGNED,IN p_revision BIGINT UNSIGNED,IN p_root BIGINT UNSIGNED)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT; DECLARE v_id BIGINT UNSIGNED DEFAULT NULL;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT a.id INTO v_id
  FROM supla_suplan_peer_association a JOIN supla_suplan_device_state s ON s.device_id=a.source_device_id AND s.user_id=a.user_id
  WHERE a.id=p_id AND a.user_id=p_user AND a.peer_generation=p_generation AND a.acl_revision=p_revision
   AND s.current_root_epoch=p_root AND p_root BETWEEN 1 AND 4294967294
   AND a.lifecycle=1 AND a.source_removed=0 AND a.destination_removed=0 FOR UPDATE;
 IF v_id IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Stale SupLAN preparation'; END IF;
 -- Validation/locking only. Root is recorded ONLY after Source OK, never before sending or forwarding credentials.
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_project(IN p_user INT, IN p_source INT, IN p_destination INT, IN p_create BOOLEAN)
SQL SECURITY DEFINER
proc: BEGIN
 DECLARE v_lock INT; DECLARE v_id BIGINT UNSIGNED DEFAULT NULL;
 DECLARE v_acl VARBINARY(534); DECLARE v_old VARBINARY(534); DECLARE v_hex TEXT;
 DECLARE v_lifecycle INT; DECLARE v_generation BIGINT UNSIGNED; DECLARE v_revision BIGINT UNSIGNED;
 DECLARE v_root BIGINT UNSIGNED DEFAULT 0; DECLARE v_provisioned BIGINT UNSIGNED; DECLARE v_count INT;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 IF v_lock IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Unknown SupLAN account'; END IF;
 SELECT id INTO v_id FROM supla_suplan_peer_association
  WHERE user_id=p_user AND source_device_id=p_source AND destination_device_id=p_destination FOR UPDATE;
 IF v_id IS NULL AND p_create AND EXISTS(
  SELECT 1 FROM supla_resource_access_grant_valid g
  JOIN supla_iodevice s ON s.id=g.source_device_id AND s.user_id=g.user_id
  JOIN supla_iodevice d ON d.id=g.destination_device_id AND d.user_id=g.user_id
  WHERE g.user_id=p_user AND g.source_device_id=p_source AND g.destination_device_id=p_destination
   AND g.peer_eligible=1 AND (s.flags & 524288)<>0 AND (d.flags & 524288)<>0
   AND s.is_virtual=0 AND d.is_virtual=0 AND s.protocol_version>=29 AND d.protocol_version>=29) THEN
  INSERT INTO supla_suplan_peer_association(user_id,source_device_id,destination_device_id,canonical_acl)
   VALUES(p_user,p_source,p_destination,X'');
  SET v_id=LAST_INSERT_ID();
 END IF;
 IF v_id IS NULL THEN LEAVE proc; END IF;
 SELECT lifecycle,peer_generation,acl_revision,canonical_acl,provisioned_root_epoch
  INTO v_lifecycle,v_generation,v_revision,v_old,v_provisioned
  FROM supla_suplan_peer_association WHERE id=v_id FOR UPDATE;
 SELECT COALESCE(MAX(current_root_epoch),0) INTO v_root FROM supla_suplan_device_state WHERE device_id=p_source AND user_id=p_user;
 SELECT COUNT(*),GROUP_CONCAT(CONCAT(LPAD(HEX(resource_type),2,'0'),LPAD(HEX(resource_id),8,'0'),
   LPAD(HEX(IF((permissions & 2)<>0,permissions & 6,permissions)),2,'0')) ORDER BY resource_type,resource_id SEPARATOR '')
 INTO v_count,v_hex FROM (
  SELECT g.resource_type,g.resource_id,BIT_OR(g.permissions) permissions FROM supla_resource_access_grant_valid g
  JOIN supla_dev_channel c ON c.id=g.resource_id AND c.user_id=g.user_id AND c.iodevice_id=g.source_device_id
   AND c.type=g.source_type AND c.func=g.source_function AND c.func<>0
  JOIN supla_iodevice s ON s.id=g.source_device_id AND s.user_id=g.user_id AND s.is_virtual=0 AND s.protocol_version>=29
  JOIN supla_iodevice d ON d.id=g.destination_device_id AND d.user_id=g.user_id AND d.is_virtual=0 AND d.protocol_version>=29
  WHERE g.user_id=p_user AND g.source_device_id=p_source AND g.destination_device_id=p_destination
   AND g.peer_eligible=1 AND (s.flags & 524288)<>0 AND (d.flags & 524288)<>0
   AND NOT EXISTS(SELECT 1 FROM supla_suplan_peer_association a WHERE a.id=v_id AND (a.source_removed=1 OR a.destination_removed=1))
  GROUP BY g.resource_type,g.resource_id FOR UPDATE
 ) rights_union;
 IF v_count>89 OR LENGTH(v_hex)<>v_count*12 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN ACL capacity exceeded'; END IF;
 SET v_acl=IFNULL(UNHEX(v_hex),X'');
 IF v_acl=v_old THEN LEAVE proc; END IF;
 IF LENGTH(v_acl)>0 AND v_lifecycle<>1 THEN
  IF v_root<>0 AND v_root=v_provisioned THEN SET v_generation=v_generation+1; END IF;
  SET v_revision=1; SET v_lifecycle=1;
 ELSEIF v_revision=4294967295 THEN SET v_generation=v_generation+1; SET v_revision=1;
 ELSE SET v_revision=v_revision+1; END IF;
 IF v_generation>4294967295 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN generation exhausted'; END IF;
 IF LENGTH(v_acl)=0 THEN SET v_lifecycle=2; END IF;
 UPDATE supla_suplan_peer_association SET lifecycle=v_lifecycle,peer_generation=v_generation,acl_revision=v_revision,
  canonical_acl=v_acl,change_revision=change_revision+1,source_empty_acked=0,destination_empty_acked=0,
  provisioned_root_epoch=IF(LENGTH(v_acl)=0 AND v_root<>0,v_root,provisioned_root_epoch) WHERE id=v_id;
 INSERT INTO supla_suplan_changed_pair(association_id,user_id,revision)
  SELECT id,user_id,change_revision FROM supla_suplan_peer_association WHERE id=v_id
  ON DUPLICATE KEY UPDATE revision=VALUES(revision);
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_resource_cleanup(IN p_user INT,IN p_channel INT)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_done BOOLEAN DEFAULT FALSE; DECLARE v_source INT; DECLARE v_destination INT; DECLARE v_lock INT;
 DECLARE v_pairs LONGTEXT DEFAULT '[]'; DECLARE v_index INT DEFAULT 0;
 DECLARE pairs CURSOR FOR
  WITH RECURSIVE slots(n) AS (SELECT 0 UNION ALL SELECT n+1 FROM slots WHERE n<88)
  SELECT a.source_device_id,a.destination_device_id FROM supla_suplan_peer_association a
   JOIN slots ON slots.n*6<LENGTH(a.canonical_acl)
   WHERE a.user_id=p_user AND SUBSTRING(a.canonical_acl,slots.n*6+1,5)=CONCAT(X'01',UNHEX(LPAD(HEX(p_channel),8,'0')))
  UNION
  SELECT source_device_id,destination_device_id FROM supla_resource_access_grant
   WHERE user_id=p_user AND origin_type=1 AND (origin_id >> 8)=p_channel
  ORDER BY source_device_id,destination_device_id;
 DECLARE CONTINUE HANDLER FOR NOT FOUND SET v_done=TRUE;
 IF @@in_transaction=0 OR EXISTS(SELECT 1 FROM supla_dev_channel WHERE id=p_channel AND user_id=p_user) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Cleanup requires deleted or foreign channel'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 -- Canonical ACL still identifies affected peers even when resource FK deletion already removed logical rows.
 OPEN pairs;
 pair_loop: LOOP
  FETCH pairs INTO v_source,v_destination;
  IF v_done THEN LEAVE pair_loop; END IF;
  SET v_pairs=JSON_ARRAY_APPEND(v_pairs,'$',JSON_OBJECT('s',v_source,'d',v_destination));
 END LOOP;
 CLOSE pairs;
 DELETE FROM supla_resource_access_grant WHERE user_id=p_user AND origin_type=1 AND (origin_id >> 8)=p_channel;
 WHILE v_index<JSON_LENGTH(v_pairs) DO
  CALL supla_suplan_project(p_user,JSON_VALUE(v_pairs,CONCAT('$[',v_index,'].s')),JSON_VALUE(v_pairs,CONCAT('$[',v_index,'].d')),FALSE);
  SET v_index=v_index+1;
 END WHILE;
END$$
CREATE OR REPLACE PROCEDURE supla_suplan_share_demand(IN p_user INT,IN p_destination INT,IN p_source_channel INT,IN p_target_channel INT)
SQL SECURITY DEFINER
BEGIN
 DECLARE v_lock INT; DECLARE v_source INT DEFAULT NULL;
 IF @@in_transaction=0 THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='SupLAN requires caller transaction'; END IF;
 SELECT id INTO v_lock FROM supla_user WHERE id=p_user FOR UPDATE;
 SELECT source_device_id INTO v_source FROM supla_resource_access_grant_valid
  WHERE user_id=p_user AND destination_device_id=p_destination AND resource_id=p_source_channel
   AND origin_type=1 AND (origin_id & 255) IN (5,6) AND binding_target_channel_id=p_target_channel
   AND peer_eligible=1 AND requires_peer=1 LIMIT 1;
 IF v_source IS NULL THEN SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='No exact Cloud Source-first binding'; END IF;
 CALL supla_suplan_peer_demand(p_user,p_destination,p_source_channel);
END$$
DELIMITER ;
