-- MySQL dump 10.17  Distrib 10.3.23-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: supla_test
-- ------------------------------------------------------
-- Server version	10.3.23-MariaDB-0+deb10u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `esp_update`
--

DROP TABLE IF EXISTS `esp_update`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `esp_update` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `device_id` int(11) NOT NULL,
  `device_name` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `platform` tinyint(4) NOT NULL,
  `latest_software_version` varchar(20) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `fparam1` int(11) NOT NULL,
  `fparam2` int(11) NOT NULL,
  `protocols` tinyint(4) NOT NULL,
  `host` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `port` int(11) NOT NULL,
  `path` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  KEY `device_name` (`device_name`),
  KEY `latest_software_version` (`latest_software_version`),
  KEY `fparam1` (`fparam1`),
  KEY `fparam2` (`fparam2`),
  KEY `platform` (`platform`),
  KEY `device_id` (`device_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `esp_update`
--

LOCK TABLES `esp_update` WRITE;
/*!40000 ALTER TABLE `esp_update` DISABLE KEYS */;
/*!40000 ALTER TABLE `esp_update` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `esp_update_log`
--

DROP TABLE IF EXISTS `esp_update_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `esp_update_log` (
  `date` datetime NOT NULL,
  `device_id` int(11) NOT NULL,
  `platform` tinyint(4) NOT NULL,
  `fparam1` int(11) NOT NULL,
  `fparam2` int(11) NOT NULL,
  `fparam3` int(11) NOT NULL,
  `fparam4` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `esp_update_log`
--

LOCK TABLES `esp_update_log` WRITE;
/*!40000 ALTER TABLE `esp_update_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `esp_update_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `migration_versions`
--

DROP TABLE IF EXISTS `migration_versions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `migration_versions` (
  `version` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `migration_versions`
--

LOCK TABLES `migration_versions` WRITE;
/*!40000 ALTER TABLE `migration_versions` DISABLE KEYS */;
INSERT INTO `migration_versions` VALUES ('20170101000000'),('20170414101854'),('20170612204116'),('20170818114139'),('20171013140904'),('20171208222022'),('20171210105120'),('20180108224520'),('20180113234138'),('20180116184415'),('20180203231115'),('20180208145738'),('20180224184251'),('20180324222844'),('20180326134725'),('20180403175932'),('20180403203101'),('20180403211558'),('20180411202101'),('20180411203913'),('20180416201401'),('20180423121539'),('20180507095139'),('20180518131234'),('20180707221458'),('20180717094843'),('20180723132652'),('20180807083217'),('20180812205513'),('20180814155501'),('20180914222230'),('20181001221229'),('20181007112610'),('20181019115859'),('20181024164957'),('20181025171850'),('20181026171557'),('20181105144611'),('20181126225634'),('20181129170610'),('20181129195431'),('20181129231132'),('20181204174603'),('20181205092324'),('20181222001450'),('20190105130410'),('20190117075805'),('20190219184847'),('20190325215115'),('20190401151822'),('20190720215803'),('20190813232026'),('20190815154016'),('20191226160845'),('20200108201101'),('20200122200601'),('20200123235701'),('20200124084227'),('20200204170901'),('20200210145902'),('20200229122103'),('20200322123636'),('20200412183701'),('20200414213205'),('20200416225304'),('20200419190150'),('20200423130550'),('20200430113342'),('20200514132030'),('20200515102311'),('20200518171230'),('20200724155001'),('20200807131101'),('20200811141801'),('20200813113801'),('20200813133501'),('20200911231401');
/*!40000 ALTER TABLE `migration_versions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_accessid`
--

DROP TABLE IF EXISTS `supla_accessid`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_accessid` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `password` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `caption` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_A5549B6CA76ED395` (`user_id`),
  CONSTRAINT `FK_A5549B6CA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_accessid`
--

LOCK TABLES `supla_accessid` WRITE;
/*!40000 ALTER TABLE `supla_accessid` DISABLE KEYS */;
INSERT INTO `supla_accessid` VALUES (2,2,'3311dbb5','Access Identifier #1',1);
/*!40000 ALTER TABLE `supla_accessid` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_amazon_alexa`
--

DROP TABLE IF EXISTS `supla_amazon_alexa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_amazon_alexa` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `reg_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `access_token` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `expires_at` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `refresh_token` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `region` varchar(5) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_290228F0A76ED395` (`user_id`),
  CONSTRAINT `FK_290228F0A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_amazon_alexa`
--

LOCK TABLES `supla_amazon_alexa` WRITE;
/*!40000 ALTER TABLE `supla_amazon_alexa` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_amazon_alexa` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_audit`
--

DROP TABLE IF EXISTS `supla_audit`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_audit` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) DEFAULT NULL,
  `event` smallint(5) unsigned NOT NULL,
  `created_at` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `text_param` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `int_param` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_EFE348F4A76ED395` (`user_id`),
  KEY `supla_audit_event_idx` (`event`),
  KEY `supla_audit_ipv4_idx` (`ipv4`),
  KEY `supla_audit_created_at_idx` (`created_at`),
  KEY `supla_audit_int_param` (`int_param`),
  CONSTRAINT `FK_EFE348F4A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=18 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_audit`
--

LOCK TABLES `supla_audit` WRITE;
/*!40000 ALTER TABLE `supla_audit` DISABLE KEYS */;
INSERT INTO `supla_audit` VALUES (1,NULL,2,'2020-04-16 08:55:27',2457392666,'',0),(2,NULL,2,'2020-04-16 08:55:32',2457392666,'',0),(3,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(4,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(5,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(6,NULL,2,'2020-04-16 08:55:46',2457392666,'',0),(7,NULL,2,'2020-04-16 14:05:47',2457392666,'test@supla.org',2),(8,NULL,7,'2020-04-16 14:06:28',NULL,'test@supla.org',1),(9,2,1,'2020-04-16 14:13:12',2457392666,'test@supla.org',NULL),(10,NULL,2,'2020-04-16 14:24:06',2457392666,'',0),(11,NULL,2,'2020-04-16 14:24:07',2457392666,'',0),(12,2,2,'2020-04-16 20:08:42',2457392666,'test@supla.org',2),(13,2,2,'2020-04-16 20:08:49',2457392666,'test@supla.org',2),(14,2,1,'2020-04-16 20:09:18',2457392666,'test@supla.org',NULL),(15,NULL,2,'2020-05-19 20:55:27',2457392666,'',0),(16,NULL,2,'2020-05-19 20:55:27',2457392666,'',0),(17,NULL,2,'2020-05-19 20:55:28',2457392666,'',0);
/*!40000 ALTER TABLE `supla_audit` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_client`
--

DROP TABLE IF EXISTS `supla_client`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_client` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `access_id` int(11) DEFAULT NULL,
  `guid` varbinary(16) NOT NULL,
  `name` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  `reg_ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `reg_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `last_access_ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `last_access_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `software_version` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `protocol_version` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `auth_key` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL,
  `caption` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `disable_after_date` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQUE_CLIENTAPP` (`user_id`,`guid`),
  KEY `IDX_5430007F4FEA67CF` (`access_id`),
  KEY `IDX_5430007FA76ED395` (`user_id`),
  CONSTRAINT `FK_5430007F4FEA67CF` FOREIGN KEY (`access_id`) REFERENCES `supla_accessid` (`id`) ON DELETE SET NULL,
  CONSTRAINT `FK_5430007FA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_client`
--

LOCK TABLES `supla_client` WRITE;
/*!40000 ALTER TABLE `supla_client` DISABLE KEYS */;
INSERT INTO `supla_client` VALUES (1,2,'\0	\n\r','Console Test Client',1,2130706433,'2020-04-16 18:21:15',2130706433,'2020-04-16 18:21:15','1.0-Linux',12,2,'$2a$04$yhIgHxTc7MFQnXTS2qdbmOWorpu7NVMR.IL1YyaJ0YPxD6P26MeaC',NULL,NULL);
/*!40000 ALTER TABLE `supla_client` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_dev_channel`
--

DROP TABLE IF EXISTS `supla_dev_channel`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_dev_channel` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `iodevice_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `channel_number` int(11) NOT NULL,
  `caption` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `type` int(11) NOT NULL,
  `func` int(11) NOT NULL,
  `flist` int(11) DEFAULT NULL,
  `param1` int(11) NOT NULL,
  `param2` int(11) NOT NULL,
  `param3` int(11) NOT NULL,
  `text_param1` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `text_param2` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `text_param3` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `alt_icon` int(11) DEFAULT NULL,
  `hidden` tinyint(1) NOT NULL DEFAULT 0,
  `location_id` int(11) DEFAULT NULL,
  `flags` int(11) DEFAULT NULL,
  `user_icon_id` int(11) DEFAULT NULL,
  `config` longtext COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQUE_CHANNEL` (`iodevice_id`,`channel_number`),
  KEY `IDX_81E928C9125F95D6` (`iodevice_id`),
  KEY `IDX_81E928C9A76ED395` (`user_id`),
  KEY `IDX_81E928C964D218E` (`location_id`),
  KEY `IDX_81E928C9CB4C938` (`user_icon_id`),
  CONSTRAINT `FK_81E928C9125F95D6` FOREIGN KEY (`iodevice_id`) REFERENCES `supla_iodevice` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_81E928C964D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`),
  CONSTRAINT `FK_81E928C9A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`),
  CONSTRAINT `FK_81E928C9CB4C938` FOREIGN KEY (`user_icon_id`) REFERENCES `supla_user_icons` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=312 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_dev_channel`
--

LOCK TABLES `supla_dev_channel` WRITE;
/*!40000 ALTER TABLE `supla_dev_channel` DISABLE KEYS */;
INSERT INTO `supla_dev_channel` VALUES (2,2,2,0,NULL,6010,410,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(140,73,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(141,73,2,1,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(142,73,2,2,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(143,73,2,3,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(144,74,2,0,'',5000,310,0,0,5600,0,'PLN',NULL,NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0.56,\"currency\":\"PLN\"}'),(146,76,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(157,83,2,0,'',3000,40,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"temperatureAdjustment\":0}'),(158,83,2,1,'',3020,45,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"humidityAdjustment\":0,\"temperatureAdjustment\":0}'),(159,83,2,2,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(160,83,2,3,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(161,83,2,4,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(162,83,2,5,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(163,83,2,6,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(164,83,2,7,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(165,83,2,8,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(166,83,2,9,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(167,83,2,10,'Szlaban',2900,20,239,500,0,0,NULL,NULL,NULL,2,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}'),(168,83,2,11,'',2900,90,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}'),(169,83,2,12,NULL,4010,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]'),(170,83,2,13,'',4000,180,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(171,83,2,14,'',4020,200,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(172,83,2,15,'',2900,130,239,0,0,0,NULL,NULL,NULL,4,0,NULL,0,NULL,'[]'),(173,83,2,16,'',2900,110,16,300,0,400,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingTimeS\":30,\"closingTimeS\":40,\"timeSettingAvailable\":true,\"openingSensorChannelId\":null}'),(174,83,2,17,'',2900,20,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}'),(175,83,2,18,'',2900,30,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}'),(176,84,2,0,'',5010,315,0,7508272,5500,1000,'PLN',NULL,NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0.55,\"impulsesPerUnit\":1000,\"currency\":\"PLN\",\"unit\":null,\"initialValue\":75082.72}'),(177,83,2,19,'',3044,260,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(178,83,2,20,'',3050,280,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(179,83,2,21,'',3042,250,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(180,83,2,22,'',3048,270,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(282,135,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(283,135,2,1,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]'),(288,84,2,1,NULL,5010,315,0,0,0,1,NULL,'impl',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"impl\",\"initialValue\":0}'),(289,84,2,2,'Overflow counter ‚ù§Ô∏è',5010,315,0,0,0,1,NULL,'x',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"x\",\"initialValue\":0}'),(290,84,2,3,NULL,5010,315,0,0,0,1,NULL,'sec',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"sec\",\"initialValue\":0}'),(291,84,2,4,NULL,5010,315,0,0,0,1,NULL,'n',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"n\",\"initialValue\":0}'),(296,84,2,5,NULL,5010,315,0,0,0,1,NULL,'Byte',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"Byte\",\"initialValue\":0}'),(301,142,2,0,NULL,7000,500,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,65536,NULL,'[]'),(303,144,2,0,NULL,8000,110,48,0,0,0,NULL,NULL,NULL,NULL,0,NULL,1,NULL,'{\"openingTimeS\":0,\"closingTimeS\":0,\"timeSettingAvailable\":true,\"openingSensorChannelId\":null}'),(308,146,2,0,NULL,2900,20,15,500,310,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"openingSensorChannelId\":310,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}'),(309,146,2,1,NULL,2900,10,15,6000,311,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"openingSensorChannelId\":311,\"relayTimeMs\":6000,\"timeSettingAvailable\":true}'),(310,146,2,2,NULL,1000,60,0,308,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"controllingChannelId\":308,\"controllingSecondaryChannelId\":null,\"invertedLogic\":false}'),(311,146,2,3,NULL,1000,50,0,309,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"controllingChannelId\":309,\"invertedLogic\":false}');
/*!40000 ALTER TABLE `supla_dev_channel` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_dev_channel_group`
--

DROP TABLE IF EXISTS `supla_dev_channel_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_dev_channel_group` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `caption` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `func` int(11) NOT NULL,
  `hidden` tinyint(1) NOT NULL DEFAULT 0,
  `location_id` int(11) NOT NULL,
  `alt_icon` int(11) DEFAULT NULL,
  `user_icon_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_6B2EFCE5A76ED395` (`user_id`),
  KEY `IDX_6B2EFCE564D218E` (`location_id`),
  KEY `IDX_6B2EFCE5CB4C938` (`user_icon_id`),
  CONSTRAINT `FK_6B2EFCE564D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`),
  CONSTRAINT `FK_6B2EFCE5A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`),
  CONSTRAINT `FK_6B2EFCE5CB4C938` FOREIGN KEY (`user_icon_id`) REFERENCES `supla_user_icons` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_dev_channel_group`
--

LOCK TABLES `supla_dev_channel_group` WRITE;
/*!40000 ALTER TABLE `supla_dev_channel_group` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_dev_channel_group` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_dev_channel_value`
--

DROP TABLE IF EXISTS `supla_dev_channel_value`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_dev_channel_value` (
  `channel_id` int(11) NOT NULL,
  `update_time` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `valid_to` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `value` varbinary(8) NOT NULL,
  `user_id` int(11) NOT NULL,
  PRIMARY KEY (`channel_id`),
  KEY `IDX_1B99E014A76ED395` (`user_id`),
  CONSTRAINT `FK_1B99E01472F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_1B99E014A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_dev_channel_value`
--

LOCK TABLES `supla_dev_channel_value` WRITE;
/*!40000 ALTER TABLE `supla_dev_channel_value` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_dev_channel_value` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_direct_link`
--

DROP TABLE IF EXISTS `supla_direct_link`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_direct_link` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `channel_id` int(11) DEFAULT NULL,
  `channel_group_id` int(11) DEFAULT NULL,
  `slug` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `caption` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `allowed_actions` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `active_from` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `active_to` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `executions_limit` int(11) DEFAULT NULL,
  `last_used` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `last_ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `enabled` tinyint(1) NOT NULL,
  `disable_http_get` tinyint(1) NOT NULL DEFAULT 0,
  `scene_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_6AE7809FA76ED395` (`user_id`),
  KEY `IDX_6AE7809F72F5A1AA` (`channel_id`),
  KEY `IDX_6AE7809F89E4AAEE` (`channel_group_id`),
  KEY `IDX_6AE7809F166053B4` (`scene_id`),
  CONSTRAINT `FK_6AE7809F166053B4` FOREIGN KEY (`scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_6AE7809F72F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_6AE7809F89E4AAEE` FOREIGN KEY (`channel_group_id`) REFERENCES `supla_dev_channel_group` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_6AE7809FA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_direct_link`
--

LOCK TABLES `supla_direct_link` WRITE;
/*!40000 ALTER TABLE `supla_direct_link` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_direct_link` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_em_log`
--

DROP TABLE IF EXISTS `supla_em_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_em_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `phase1_fae` bigint(20) DEFAULT NULL,
  `phase1_rae` bigint(20) DEFAULT NULL,
  `phase1_fre` bigint(20) DEFAULT NULL,
  `phase1_rre` bigint(20) DEFAULT NULL,
  `phase2_fae` bigint(20) DEFAULT NULL,
  `phase2_rae` bigint(20) DEFAULT NULL,
  `phase2_fre` bigint(20) DEFAULT NULL,
  `phase2_rre` bigint(20) DEFAULT NULL,
  `phase3_fae` bigint(20) DEFAULT NULL,
  `phase3_rae` bigint(20) DEFAULT NULL,
  `phase3_fre` bigint(20) DEFAULT NULL,
  `phase3_rre` bigint(20) DEFAULT NULL,
  `fae_balanced` bigint(20) DEFAULT NULL,
  `rae_balanced` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_em_log`
--

LOCK TABLES `supla_em_log` WRITE;
/*!40000 ALTER TABLE `supla_em_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_em_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_google_home`
--

DROP TABLE IF EXISTS `supla_google_home`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_google_home` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `reg_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `access_token` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_98090074A76ED395` (`user_id`),
  CONSTRAINT `FK_98090074A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_google_home`
--

LOCK TABLES `supla_google_home` WRITE;
/*!40000 ALTER TABLE `supla_google_home` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_google_home` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_ic_log`
--

DROP TABLE IF EXISTS `supla_ic_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_ic_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `counter` bigint(20) NOT NULL,
  `calculated_value` bigint(20) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_ic_log`
--

LOCK TABLES `supla_ic_log` WRITE;
/*!40000 ALTER TABLE `supla_ic_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_ic_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_iodevice`
--

DROP TABLE IF EXISTS `supla_iodevice`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_iodevice` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `guid` varbinary(16) NOT NULL,
  `name` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  `comment` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `reg_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `reg_ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `last_connected` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `last_ipv4` int(10) unsigned DEFAULT NULL COMMENT '(DC2Type:ipaddress)',
  `software_version` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `protocol_version` int(11) NOT NULL,
  `original_location_id` int(11) DEFAULT NULL,
  `auth_key` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL,
  `flags` int(11) DEFAULT NULL,
  `manufacturer_id` smallint(6) DEFAULT NULL,
  `product_id` smallint(6) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQUE_USER_GUID` (`user_id`,`guid`),
  KEY `IDX_793D49D64D218E` (`location_id`),
  KEY `IDX_793D49DA76ED395` (`user_id`),
  KEY `IDX_793D49DF142C1A4` (`original_location_id`),
  CONSTRAINT `FK_793D49D64D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`),
  CONSTRAINT `FK_793D49DA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`),
  CONSTRAINT `FK_793D49DF142C1A4` FOREIGN KEY (`original_location_id`) REFERENCES `supla_location` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=147 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_iodevice`
--

LOCK TABLES `supla_iodevice` WRITE;
/*!40000 ALTER TABLE `supla_iodevice` DISABLE KEYS */;
INSERT INTO `supla_iodevice` VALUES (2,2,2,'+Ø _∫∞≤Ú\"“¢ÑY¬]N','HEATPOL HOME+',1,NULL,'2019-02-25 10:12:48',2457392666,'2019-02-27 12:08:08',2457392666,'2.7.4',11,NULL,'$2a$04$ZF0woWUjKtVluzQ5LQ4M8e1lBYN.Qne20EK.O/tFVItZL9QzYviM6',0,0,0),(73,2,2,'¸µÎg∞çÔøÕ%±[Õ/','ZAMEL ROW-04m',1,NULL,'2019-08-06 10:45:49',2457392666,'2019-12-03 12:23:30',2457392666,'2.7.5',7,NULL,'$2a$04$UMltnzqt78XvZkfo.PNUH.BrPk6ycsmJesV1cfpSG7afIEyEsoFgO',0,4,0),(74,2,2,'|èÍm˘ÔRP„î.{îÍº','ZAMEL MEW-01',1,NULL,'2019-08-06 10:48:09',2457392666,'2020-04-16 14:07:33',2457392666,'2.7.10',10,NULL,'$2a$04$VkijuJV2SYMeDvD3P0ObT.qzEitqHeciTOsY8xVadLx7c1VrOXajS',0,4,1000),(76,2,2,'˝ÎßE#ò}Êíyó*˛≈Xí','ZAMEL ROW-16A',1,NULL,'2019-09-11 10:49:23',2457392666,'2019-10-24 14:23:35',2457392666,'2.7.10',7,NULL,'$2a$04$q4FXGId6iPdd6R7Xm2bGhONeOXfxlT0Ekfzhw33rpu00K7ECoI06K',0,4,0),(83,2,2,'·I«ﬂÒ!ﬁπŒ.5','ARDUINO',1,NULL,'2019-10-17 10:20:52',2457392666,'2019-11-21 18:19:08',2457392666,'1.6.1',11,NULL,NULL,0,0,0),(84,2,2,'Ìü=◊2OIÂúﬂw¸(','ZAMEL LIW-01',1,NULL,'2019-10-23 09:05:04',2457392666,'2020-04-16 14:06:32',2457392666,'2.7.16',10,NULL,'$2a$04$AsyFsH3lcFOno0UvpzkIX.uSoTMq/bg4cYi7oJygbqnCkEcIRfWjy',0,4,1010),(135,2,2,'n>∫]#\"ÌÿWE_{ìëé','ZAMEL ROW-02m',1,NULL,'2019-11-06 11:34:47',2457392666,'2019-11-06 11:35:15',2457392666,'2.7.13',7,NULL,'$2a$04$X13wWD0Y0qgQmf/HV.UeOO5V1t2lQhdzJ7DZv89Nt.uQW1tTavWnK',0,4,0),(142,2,2,'Ñîz—Rõî#:« ”™.','PEVEKO VALVE',1,NULL,'2020-01-28 16:43:24',2457392666,'2020-01-28 19:44:35',2457392666,'2.7.16',11,NULL,'$2a$04$gccY4TpEEbPYRavhkWugiu8.jglxjjW5rZ.kvtLCSyP3ZgoMcsesa',0,10,1),(144,2,2,'ÂeŸ˜‘j¿ﬂúãT˝@¨ô','FAKRO BRIDGE',1,NULL,'2020-02-19 20:25:39',2457392666,'2020-03-13 23:53:48',1332661374,'2.7.16',12,NULL,'$2a$04$QFl5vYuSP6b5Tzowv4odAugs92Fr0aO.5BizMmaxpe2DAb/jh63Q.',0,9,1),(146,2,2,'ﬁ—◊’)>]ºÒ}€ı5L','ZAMEL SBW-02',1,NULL,'2020-03-01 13:35:51',1332661374,'2020-03-03 20:04:14',1332661374,'2.7.16',7,NULL,'$2a$04$k5z2Tv6fEfRVfvTNAczLX.LqPO.6OKA.hU4/PH.xU4LGtFfazutG6',0,4,0);
/*!40000 ALTER TABLE `supla_iodevice` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_location`
--

DROP TABLE IF EXISTS `supla_location`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_location` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `password` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `caption` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_3698128EA76ED395` (`user_id`),
  CONSTRAINT `FK_3698128EA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_location`
--

LOCK TABLES `supla_location` WRITE;
/*!40000 ALTER TABLE `supla_location` DISABLE KEYS */;
INSERT INTO `supla_location` VALUES (2,2,'8aea','Location #1',1);
/*!40000 ALTER TABLE `supla_location` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_oauth_access_tokens`
--

DROP TABLE IF EXISTS `supla_oauth_access_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_oauth_access_tokens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `client_id` int(11) DEFAULT NULL,
  `user_id` int(11) DEFAULT NULL,
  `token` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `expires_at` int(11) DEFAULT NULL,
  `scope` varchar(2000) COLLATE utf8_unicode_ci NOT NULL,
  `name` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  `access_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_2402564B5F37A13B` (`token`),
  KEY `IDX_2402564B19EB6921` (`client_id`),
  KEY `IDX_2402564BA76ED395` (`user_id`),
  KEY `IDX_2402564B4FEA67CF` (`access_id`),
  CONSTRAINT `FK_2402564B19EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_2402564B4FEA67CF` FOREIGN KEY (`access_id`) REFERENCES `supla_accessid` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_2402564BA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_access_tokens`
--

LOCK TABLES `supla_oauth_access_tokens` WRITE;
/*!40000 ALTER TABLE `supla_oauth_access_tokens` DISABLE KEYS */;
INSERT INTO `supla_oauth_access_tokens` VALUES (1,1,2,'ZWE2ZWQ5NDVkMTQ5Y2UxYTI3ODdmOTVkYjE5ODEwMjcxZDZjM2U0ZDA2OWMxZjg0ZGI0NDY3MjNmNDdkYzQyZQ.aHR0cHM6Ly9kZXZlbC1jbG91ZC5zdXBsYS5vcmc=',1587047672,'channels_ea channelgroups_ea scenes_ea channels_files accessids_r accessids_rw account_r account_rw channels_r channels_rw channelgroups_r channelgroups_rw clientapps_r clientapps_rw directlinks_r directlinks_rw iodevices_r iodevices_rw locations_r locations_rw scenes_r scenes_rw schedules_r schedules_rw',NULL,NULL),(2,1,2,'ZWNmMWQ1OTY5ZDM2YTc3YTU0MDNjMzQ3MjAzMjBkNjkxMjIwMjM1NDNmNmUyNDc1ZDAzNThjODlhZGUyZTg1MA.aHR0cHM6Ly9kZXZlbC1jbG91ZC5zdXBsYS5vcmc=',1587047672,'channels_files',NULL,NULL),(3,1,2,'ZGZhYzJlMjY4YzQ5MzkwNTFmZTI4ODEyYzIwZmU4ZGQwNGY3ZTc2NzVhMTUzZmFlZjA3ZWMyZTQ5NDcwZTRhMQ.aHR0cHM6Ly9kZXZlbC1jbG91ZC5zdXBsYS5vcmc=',1587069003,'channels_ea channelgroups_ea scenes_ea channels_files accessids_r accessids_rw account_r account_rw channels_r channels_rw channelgroups_r channelgroups_rw clientapps_r clientapps_rw directlinks_r directlinks_rw iodevices_r iodevices_rw locations_r locations_rw scenes_r scenes_rw schedules_r schedules_rw',NULL,NULL),(4,1,2,'MjhjMDg3NTRiMjExM2Y0ZmRkYWUzMjQ0YzM4MGVkY2E5MTMwMGUzOGQwM2RjNzk3NTg1MWVkYjFjOTkzOGJjZg.aHR0cHM6Ly9kZXZlbC1jbG91ZC5zdXBsYS5vcmc=',1587069003,'channels_files',NULL,NULL);
/*!40000 ALTER TABLE `supla_oauth_access_tokens` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_oauth_auth_codes`
--

DROP TABLE IF EXISTS `supla_oauth_auth_codes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_oauth_auth_codes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `client_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `token` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `redirect_uri` longtext COLLATE utf8_unicode_ci NOT NULL,
  `expires_at` int(11) DEFAULT NULL,
  `scope` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_48E00E5D5F37A13B` (`token`),
  KEY `IDX_48E00E5D19EB6921` (`client_id`),
  KEY `IDX_48E00E5DA76ED395` (`user_id`),
  CONSTRAINT `FK_48E00E5D19EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_48E00E5DA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_auth_codes`
--

LOCK TABLES `supla_oauth_auth_codes` WRITE;
/*!40000 ALTER TABLE `supla_oauth_auth_codes` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_oauth_auth_codes` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_oauth_client_authorizations`
--

DROP TABLE IF EXISTS `supla_oauth_client_authorizations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_oauth_client_authorizations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `client_id` int(11) NOT NULL,
  `scope` varchar(2000) COLLATE utf8_unicode_ci NOT NULL,
  `authorization_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQUE_USER_CLIENT` (`user_id`,`client_id`),
  KEY `IDX_6B787396A76ED395` (`user_id`),
  KEY `IDX_6B78739619EB6921` (`client_id`),
  CONSTRAINT `FK_6B78739619EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_6B787396A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_client_authorizations`
--

LOCK TABLES `supla_oauth_client_authorizations` WRITE;
/*!40000 ALTER TABLE `supla_oauth_client_authorizations` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_oauth_client_authorizations` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_oauth_clients`
--

DROP TABLE IF EXISTS `supla_oauth_clients`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_oauth_clients` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `random_id` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `redirect_uris` longtext COLLATE utf8_unicode_ci NOT NULL COMMENT '(DC2Type:array)',
  `secret` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `allowed_grant_types` longtext COLLATE utf8_unicode_ci NOT NULL COMMENT '(DC2Type:array)',
  `type` int(11) NOT NULL,
  `user_id` int(11) DEFAULT NULL,
  `name` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `description` longtext COLLATE utf8_unicode_ci DEFAULT NULL,
  `public_client_id` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `long_description` longtext COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_4035AD80A76ED395` (`user_id`),
  KEY `supla_oauth_clients_random_id_idx` (`random_id`),
  KEY `supla_oauth_clients_type_idx` (`type`),
  CONSTRAINT `FK_4035AD80A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_clients`
--

LOCK TABLES `supla_oauth_clients` WRITE;
/*!40000 ALTER TABLE `supla_oauth_clients` DISABLE KEYS */;
INSERT INTO `supla_oauth_clients` VALUES (1,'60xt2173qw84s8s80kock480scos4ck884o844k0sg8kw8g8o0','a:0:{}','ohrnj61b63kgcooo084kcsosowcs4kgggow8owokw8s0w4c04','a:2:{i:0;s:8:\"password\";i:1;s:13:\"refresh_token\";}',1,NULL,NULL,NULL,NULL,NULL);
/*!40000 ALTER TABLE `supla_oauth_clients` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_oauth_refresh_tokens`
--

DROP TABLE IF EXISTS `supla_oauth_refresh_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_oauth_refresh_tokens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `client_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `token` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `expires_at` int(11) DEFAULT NULL,
  `scope` varchar(2000) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_B809538C5F37A13B` (`token`),
  KEY `IDX_B809538C19EB6921` (`client_id`),
  KEY `IDX_B809538CA76ED395` (`user_id`),
  CONSTRAINT `FK_B809538C19EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_B809538CA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_refresh_tokens`
--

LOCK TABLES `supla_oauth_refresh_tokens` WRITE;
/*!40000 ALTER TABLE `supla_oauth_refresh_tokens` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_oauth_refresh_tokens` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_rel_aidloc`
--

DROP TABLE IF EXISTS `supla_rel_aidloc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_rel_aidloc` (
  `location_id` int(11) NOT NULL,
  `access_id` int(11) NOT NULL,
  PRIMARY KEY (`location_id`,`access_id`),
  KEY `IDX_2B15904164D218E` (`location_id`),
  KEY `IDX_2B1590414FEA67CF` (`access_id`),
  CONSTRAINT `FK_2B1590414FEA67CF` FOREIGN KEY (`access_id`) REFERENCES `supla_accessid` (`id`),
  CONSTRAINT `FK_2B15904164D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_rel_aidloc`
--

LOCK TABLES `supla_rel_aidloc` WRITE;
/*!40000 ALTER TABLE `supla_rel_aidloc` DISABLE KEYS */;
INSERT INTO `supla_rel_aidloc` VALUES (2,2);
/*!40000 ALTER TABLE `supla_rel_aidloc` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_rel_cg`
--

DROP TABLE IF EXISTS `supla_rel_cg`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_rel_cg` (
  `channel_id` int(11) NOT NULL,
  `group_id` int(11) NOT NULL,
  PRIMARY KEY (`group_id`,`channel_id`),
  KEY `IDX_BE981CD772F5A1AA` (`channel_id`),
  KEY `IDX_BE981CD7FE54D947` (`group_id`),
  CONSTRAINT `FK_BE981CD772F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`),
  CONSTRAINT `FK_BE981CD7FE54D947` FOREIGN KEY (`group_id`) REFERENCES `supla_dev_channel_group` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_rel_cg`
--

LOCK TABLES `supla_rel_cg` WRITE;
/*!40000 ALTER TABLE `supla_rel_cg` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_rel_cg` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_scene`
--

DROP TABLE IF EXISTS `supla_scene`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_scene` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `location_id` int(11) NOT NULL,
  `caption` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  `user_icon_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_A4825857A76ED395` (`user_id`),
  KEY `IDX_A482585764D218E` (`location_id`),
  KEY `IDX_A4825857CB4C938` (`user_icon_id`),
  CONSTRAINT `FK_A482585764D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`),
  CONSTRAINT `FK_A4825857A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`),
  CONSTRAINT `FK_A4825857CB4C938` FOREIGN KEY (`user_icon_id`) REFERENCES `supla_user_icons` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_scene`
--

LOCK TABLES `supla_scene` WRITE;
/*!40000 ALTER TABLE `supla_scene` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_scene` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_scene_operation`
--

DROP TABLE IF EXISTS `supla_scene_operation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_scene_operation` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `owning_scene_id` int(11) NOT NULL,
  `channel_id` int(11) DEFAULT NULL,
  `channel_group_id` int(11) DEFAULT NULL,
  `scene_id` int(11) DEFAULT NULL,
  `action` int(11) NOT NULL,
  `action_param` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `delay_ms` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `IDX_64A50CF5E019BC26` (`owning_scene_id`),
  KEY `IDX_64A50CF572F5A1AA` (`channel_id`),
  KEY `IDX_64A50CF589E4AAEE` (`channel_group_id`),
  KEY `IDX_64A50CF5166053B4` (`scene_id`),
  CONSTRAINT `FK_64A50CF5166053B4` FOREIGN KEY (`scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF572F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF589E4AAEE` FOREIGN KEY (`channel_group_id`) REFERENCES `supla_dev_channel_group` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF5E019BC26` FOREIGN KEY (`owning_scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_scene_operation`
--

LOCK TABLES `supla_scene_operation` WRITE;
/*!40000 ALTER TABLE `supla_scene_operation` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_scene_operation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_schedule`
--

DROP TABLE IF EXISTS `supla_schedule`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_schedule` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `channel_id` int(11) DEFAULT NULL,
  `time_expression` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `action` int(11) NOT NULL,
  `action_param` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `mode` varchar(15) COLLATE utf8_unicode_ci NOT NULL,
  `date_start` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `date_end` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `enabled` tinyint(1) NOT NULL,
  `next_calculation_date` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `caption` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `retry` tinyint(1) NOT NULL DEFAULT 1,
  `channel_group_id` int(11) DEFAULT NULL,
  `scene_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_323E8ABEA76ED395` (`user_id`),
  KEY `IDX_323E8ABE72F5A1AA` (`channel_id`),
  KEY `next_calculation_date_idx` (`next_calculation_date`),
  KEY `enabled_idx` (`enabled`),
  KEY `date_start_idx` (`date_start`),
  KEY `IDX_323E8ABE89E4AAEE` (`channel_group_id`),
  KEY `IDX_323E8ABE166053B4` (`scene_id`),
  CONSTRAINT `FK_323E8ABE166053B4` FOREIGN KEY (`scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_323E8ABE72F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_323E8ABE89E4AAEE` FOREIGN KEY (`channel_group_id`) REFERENCES `supla_dev_channel_group` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_323E8ABEA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_schedule`
--

LOCK TABLES `supla_schedule` WRITE;
/*!40000 ALTER TABLE `supla_schedule` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_schedule` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_scheduled_executions`
--

DROP TABLE IF EXISTS `supla_scheduled_executions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_scheduled_executions` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `schedule_id` int(11) NOT NULL,
  `planned_timestamp` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `fetched_timestamp` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `retry_timestamp` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `retry_count` int(11) DEFAULT NULL,
  `result_timestamp` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `consumed` tinyint(1) NOT NULL,
  `result` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_FB21DBDCA40BC2D5` (`schedule_id`),
  KEY `result_idx` (`result`),
  KEY `result_timestamp_idx` (`result_timestamp`),
  KEY `planned_timestamp_idx` (`planned_timestamp`),
  KEY `retry_timestamp_idx` (`retry_timestamp`),
  KEY `fetched_timestamp_idx` (`fetched_timestamp`),
  KEY `consumed_idx` (`consumed`),
  CONSTRAINT `FK_FB21DBDCA40BC2D5` FOREIGN KEY (`schedule_id`) REFERENCES `supla_schedule` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_scheduled_executions`
--

LOCK TABLES `supla_scheduled_executions` WRITE;
/*!40000 ALTER TABLE `supla_scheduled_executions` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_scheduled_executions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_state_webhooks`
--

DROP TABLE IF EXISTS `supla_state_webhooks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_state_webhooks` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `client_id` int(11) DEFAULT NULL,
  `user_id` int(11) DEFAULT NULL,
  `url` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `access_token` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `refresh_token` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `expires_at` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `functions_ids` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  KEY `IDX_3C9361E019EB6921` (`client_id`),
  KEY `IDX_3C9361E0A76ED395` (`user_id`),
  CONSTRAINT `FK_3C9361E019EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_3C9361E0A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_state_webhooks`
--

LOCK TABLES `supla_state_webhooks` WRITE;
/*!40000 ALTER TABLE `supla_state_webhooks` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_state_webhooks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_temperature_log`
--

DROP TABLE IF EXISTS `supla_temperature_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_temperature_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `temperature` decimal(8,4) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_temperature_log`
--

LOCK TABLES `supla_temperature_log` WRITE;
/*!40000 ALTER TABLE `supla_temperature_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_temperature_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_temphumidity_log`
--

DROP TABLE IF EXISTS `supla_temphumidity_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_temphumidity_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `temperature` decimal(8,4) NOT NULL,
  `humidity` decimal(8,4) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_temphumidity_log`
--

LOCK TABLES `supla_temphumidity_log` WRITE;
/*!40000 ALTER TABLE `supla_temphumidity_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_temphumidity_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_thermostat_log`
--

DROP TABLE IF EXISTS `supla_thermostat_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_thermostat_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `on` tinyint(1) NOT NULL,
  `measured_temperature` decimal(5,2) NOT NULL,
  `preset_temperature` decimal(5,2) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_thermostat_log`
--

LOCK TABLES `supla_thermostat_log` WRITE;
/*!40000 ALTER TABLE `supla_thermostat_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_thermostat_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_user`
--

DROP TABLE IF EXISTS `supla_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `short_unique_id` char(32) COLLATE utf8_unicode_ci NOT NULL,
  `long_unique_id` char(200) COLLATE utf8_unicode_ci NOT NULL,
  `salt` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `email` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `password` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  `reg_date` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `token` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `password_requested_at` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `limit_aid` int(11) NOT NULL,
  `limit_loc` int(11) NOT NULL,
  `limit_iodev` int(11) NOT NULL,
  `limit_client` int(11) NOT NULL,
  `timezone` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `limit_schedule` int(11) NOT NULL DEFAULT 20,
  `legacy_password` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL,
  `iodevice_reg_enabled` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `client_reg_enabled` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `limit_channel_group` int(11) NOT NULL DEFAULT 20,
  `limit_channel_per_group` int(11) NOT NULL DEFAULT 10,
  `rules_agreement` tinyint(1) NOT NULL DEFAULT 0,
  `cookies_agreement` tinyint(1) NOT NULL DEFAULT 0,
  `oauth_compat_username` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'For backward compatibility purpose',
  `oauth_compat_password` varchar(64) COLLATE utf8_unicode_ci DEFAULT NULL COMMENT 'For backward compatibility purpose',
  `limit_direct_link` int(11) NOT NULL DEFAULT 50,
  `limit_oauth_client` int(11) NOT NULL DEFAULT 20,
  `locale` varchar(5) COLLATE utf8_unicode_ci DEFAULT NULL,
  `account_removal_requested_at` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `limit_scene` int(11) NOT NULL DEFAULT 50,
  `api_rate_limit` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_71BAEAC6E7927C74` (`email`),
  UNIQUE KEY `UNIQ_71BAEAC69DAF5974` (`short_unique_id`),
  UNIQUE KEY `UNIQ_71BAEAC6AB4C1E2D` (`long_unique_id`),
  KEY `client_reg_enabled_idx` (`client_reg_enabled`),
  KEY `iodevice_reg_enabled_idx` (`iodevice_reg_enabled`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_user`
--

LOCK TABLES `supla_user` WRITE;
/*!40000 ALTER TABLE `supla_user` DISABLE KEYS */;
INSERT INTO `supla_user` VALUES (2,'6d4e39883a09d3baffe9e15b5fedc152','090874ade7b4e23aef2330e81cd4784cc3badcb0b6df34c8a0be0d91ded0b5567fde8b36bf920c5a2e1ead39aa4494d3d262a1cc4a6a93225a15fec85265858f85936af20236cb7b50a8e9a420897c966cbe22f2e55275f033b6b2b10e2b5136bcb31fcb','af52uahn388cg0kggw4008swkgc0s04','test@supla.org','$2y$13$991OYKopr/P1IPP6AvarJ.12b7edDIeuRSxz/xJKrf4K1RFrvZxwu',1,'2020-04-16 14:07:33',NULL,NULL,10,10,100,200,'Europe/Berlin',20,NULL,NULL,NULL,20,10,1,0,NULL,NULL,50,20,'pl',NULL,50,NULL);
/*!40000 ALTER TABLE `supla_user` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_user_icons`
--

DROP TABLE IF EXISTS `supla_user_icons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_user_icons` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `func` int(11) NOT NULL,
  `image1` longblob NOT NULL,
  `image2` longblob DEFAULT NULL,
  `image3` longblob DEFAULT NULL,
  `image4` longblob DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_27B32ACA76ED395` (`user_id`),
  CONSTRAINT `FK_EEB07467A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_user_icons`
--

LOCK TABLES `supla_user_icons` WRITE;
/*!40000 ALTER TABLE `supla_user_icons` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_user_icons` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Temporary table structure for view `supla_v_client`
--

DROP TABLE IF EXISTS `supla_v_client`;
/*!50001 DROP VIEW IF EXISTS `supla_v_client`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_client` (
  `id` tinyint NOT NULL,
  `access_id` tinyint NOT NULL,
  `guid` tinyint NOT NULL,
  `name` tinyint NOT NULL,
  `reg_ipv4` tinyint NOT NULL,
  `reg_date` tinyint NOT NULL,
  `last_access_ipv4` tinyint NOT NULL,
  `last_access_date` tinyint NOT NULL,
  `software_version` tinyint NOT NULL,
  `protocol_version` tinyint NOT NULL,
  `enabled` tinyint NOT NULL,
  `user_id` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_client_channel`
--

DROP TABLE IF EXISTS `supla_v_client_channel`;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_channel`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_client_channel` (
  `id` tinyint NOT NULL,
  `type` tinyint NOT NULL,
  `func` tinyint NOT NULL,
  `param1` tinyint NOT NULL,
  `param2` tinyint NOT NULL,
  `caption` tinyint NOT NULL,
  `param3` tinyint NOT NULL,
  `text_param1` tinyint NOT NULL,
  `text_param2` tinyint NOT NULL,
  `text_param3` tinyint NOT NULL,
  `manufacturer_id` tinyint NOT NULL,
  `product_id` tinyint NOT NULL,
  `user_icon_id` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `channel_number` tinyint NOT NULL,
  `iodevice_id` tinyint NOT NULL,
  `client_id` tinyint NOT NULL,
  `location_id` tinyint NOT NULL,
  `alt_icon` tinyint NOT NULL,
  `protocol_version` tinyint NOT NULL,
  `flags` tinyint NOT NULL,
  `value` tinyint NOT NULL,
  `validity_time_sec` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_client_channel_group`
--

DROP TABLE IF EXISTS `supla_v_client_channel_group`;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_channel_group`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_client_channel_group` (
  `id` tinyint NOT NULL,
  `func` tinyint NOT NULL,
  `caption` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `location_id` tinyint NOT NULL,
  `alt_icon` tinyint NOT NULL,
  `user_icon_id` tinyint NOT NULL,
  `client_id` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_client_location`
--

DROP TABLE IF EXISTS `supla_v_client_location`;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_location`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_client_location` (
  `id` tinyint NOT NULL,
  `caption` tinyint NOT NULL,
  `client_id` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_device_accessid`
--

DROP TABLE IF EXISTS `supla_v_device_accessid`;
/*!50001 DROP VIEW IF EXISTS `supla_v_device_accessid`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_device_accessid` (
  `id` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `enabled` tinyint NOT NULL,
  `password` tinyint NOT NULL,
  `limit_client` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_device_location`
--

DROP TABLE IF EXISTS `supla_v_device_location`;
/*!50001 DROP VIEW IF EXISTS `supla_v_device_location`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_device_location` (
  `id` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `enabled` tinyint NOT NULL,
  `limit_iodev` tinyint NOT NULL,
  `password` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_rel_cg`
--

DROP TABLE IF EXISTS `supla_v_rel_cg`;
/*!50001 DROP VIEW IF EXISTS `supla_v_rel_cg`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_rel_cg` (
  `group_id` tinyint NOT NULL,
  `channel_id` tinyint NOT NULL,
  `iodevice_id` tinyint NOT NULL,
  `protocol_version` tinyint NOT NULL,
  `client_id` tinyint NOT NULL,
  `channel_hidden` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_user_channel_group`
--

DROP TABLE IF EXISTS `supla_v_user_channel_group`;
/*!50001 DROP VIEW IF EXISTS `supla_v_user_channel_group`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_user_channel_group` (
  `id` tinyint NOT NULL,
  `func` tinyint NOT NULL,
  `caption` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `location_id` tinyint NOT NULL,
  `alt_icon` tinyint NOT NULL,
  `channel_id` tinyint NOT NULL,
  `iodevice_id` tinyint NOT NULL,
  `hidden` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Dumping routines for database 'supla_test'
--
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_channel` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_channel`(IN `_type` INT, IN `_func` INT, IN `_param1` INT, IN `_param2` INT, IN `_param3` INT, 
IN `_user_id` INT, IN `_channel_number` INT, IN `_iodevice_id` INT, IN `_flist` INT, IN `_flags` INT)
    NO SQL
BEGIN

INSERT INTO `supla_dev_channel` (`type`, `func`, `param1`, `param2`, `param3`, `user_id`, `channel_number`, 
`iodevice_id`, `flist`, `flags`) 
VALUES (_type, _func, _param1, _param2, _param3, _user_id, _channel_number, _iodevice_id, _flist, _flags);

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_client` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_client`(IN `_access_id` INT(11), IN `_guid` VARBINARY(16), IN `_name` VARCHAR(100) CHARSET utf8mb4, IN `_reg_ipv4` INT(10) UNSIGNED, IN `_software_version` VARCHAR(20) CHARSET utf8, IN `_protocol_version` INT(11), IN `_user_id` INT(11), IN `_auth_key` VARCHAR(64) CHARSET utf8, OUT `_id` INT(11))
    NO SQL
BEGIN

IF EXISTS(SELECT 1 FROM `supla_user` WHERE `id` = _user_id) THEN

INSERT INTO `supla_client`(`access_id`, `guid`, `name`, `enabled`, `reg_ipv4`, `reg_date`, `last_access_ipv4`,
`last_access_date`,`software_version`, `protocol_version`, `user_id`, `auth_key`)
VALUES (_access_id, _guid, _name, 1, _reg_ipv4, UTC_TIMESTAMP(), _reg_ipv4, UTC_TIMESTAMP(), _software_version, _protocol_version,
    _user_id, _auth_key);

SELECT LAST_INSERT_ID() INTO _id;

END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_em_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_em_log_item`(IN `_channel_id` INT(11), IN `_phase1_fae` BIGINT, IN `_phase1_rae` BIGINT, 
IN `_phase1_fre` BIGINT, IN `_phase1_rre` BIGINT, IN `_phase2_fae` BIGINT, IN `_phase2_rae` BIGINT, IN `_phase2_fre` BIGINT, 
IN `_phase2_rre` BIGINT, IN `_phase3_fae` BIGINT, IN `_phase3_rae` BIGINT, IN `_phase3_fre` BIGINT, IN `_phase3_rre` BIGINT,
IN `_fae_balanced` BIGINT, IN `_rae_balanced` BIGINT)
    NO SQL
BEGIN

INSERT INTO `supla_em_log`(`channel_id`, `date`, `phase1_fae`, `phase1_rae`, `phase1_fre`, `phase1_rre`, `phase2_fae`, 
`phase2_rae`, `phase2_fre`, `phase2_rre`, `phase3_fae`, `phase3_rae`, `phase3_fre`, `phase3_rre`, `fae_balanced`, `rae_balanced`) 
VALUES (_channel_id, UTC_TIMESTAMP(), _phase1_fae, _phase1_rae, _phase1_fre, _phase1_rre, 
_phase2_fae, _phase2_rae, _phase2_fre, _phase2_rre, _phase3_fae, _phase3_rae, _phase3_fre, _phase3_rre,
_fae_balanced, _rae_balanced);

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_ic_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_ic_log_item`(IN `_channel_id` INT(11), IN `_counter` BIGINT, IN `_calculated_value` BIGINT)
    NO SQL
BEGIN

INSERT INTO `supla_ic_log`(`channel_id`, `date`, `counter`, `calculated_value`) 
VALUES (_channel_id,UTC_TIMESTAMP(),_counter, _calculated_value);

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_iodevice` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_iodevice`(IN `_location_id` INT(11), IN `_user_id` INT(11), IN `_guid` VARBINARY(16), IN `_name` VARCHAR(100) CHARSET utf8mb4, IN `_reg_ipv4` INT(10) UNSIGNED, IN `_software_version` VARCHAR(20), IN `_protocol_version` INT(11), IN `_product_id` SMALLINT, IN `_manufacturer_id` SMALLINT, IN `_original_location_id` INT(11), IN `_auth_key` VARCHAR(64), IN `_flags` INT(11), OUT `_id` INT(11))
    NO SQL
BEGIN
SET @mfr_id = _manufacturer_id;
IF _manufacturer_id = 0 THEN
  IF _name LIKE '%sonoff%' THEN SELECT 6 INTO @mfr_id; END IF;
  IF _name LIKE 'NICE %' THEN SELECT  5 INTO @mfr_id; END IF;
  IF _name LIKE 'ZAMEL %' THEN SELECT 4 INTO @mfr_id; END IF;
END IF;
INSERT INTO `supla_iodevice`(`location_id`, `user_id`, `guid`, `name`, `enabled`, `reg_date`, `reg_ipv4`, `last_connected`, `last_ipv4`,
`software_version`, `protocol_version`, `manufacturer_id`, `product_id`, `original_location_id`, `auth_key`, `flags`)
VALUES (_location_id, _user_id, _guid, _name, 1, UTC_TIMESTAMP(), _reg_ipv4, UTC_TIMESTAMP(), _reg_ipv4, _software_version,
    _protocol_version, @mfr_id, _product_id, _original_location_id, _auth_key, _flags);
SELECT LAST_INSERT_ID() INTO _id;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_temperature_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_temperature_log_item`(IN `_channel_id` INT(11), IN `_temperature` DECIMAL(8,4))
    NO SQL
BEGIN

INSERT INTO `supla_temperature_log`(`channel_id`, `date`, `temperature`) VALUES (_channel_id,UTC_TIMESTAMP(),_temperature);

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_temphumidity_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_temphumidity_log_item`(IN `_channel_id` INT(11), IN `_temperature` DECIMAL(8,4), 
IN `_humidity` DECIMAL(8,4))
    NO SQL
BEGIN

INSERT INTO `supla_temphumidity_log`(`channel_id`, `date`, `temperature`, `humidity`) 
VALUES (_channel_id,UTC_TIMESTAMP(),_temperature, _humidity);

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_thermostat_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_thermostat_log_item`(IN `_channel_id` INT(11), IN `_measured_temperature` DECIMAL(5,2), IN `_preset_temperature` DECIMAL(5,2), IN `_on` TINYINT)
    NO SQL
BEGIN INSERT INTO `supla_thermostat_log`(`channel_id`, `date`, `measured_temperature`, `preset_temperature`, `on`) VALUES (_channel_id,UTC_TIMESTAMP(),_measured_temperature, _preset_temperature, _on); END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_get_device_firmware_url` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_get_device_firmware_url`(IN `device_id` INT, IN `platform` TINYINT, IN `fparam1` INT, IN `fparam2` INT, IN `fparam3` INT, IN `fparam4` INT, OUT `protocols` TINYINT, OUT `host` VARCHAR(100), OUT `port` INT, OUT `path` VARCHAR(100))
    NO SQL
BEGIN
                SET @protocols = 0;
                SET @host = '';
                SET @port = 0;
                SET @path = '';
                
                SET @fparam1 = fparam1;
                SET @fparam2 = fparam2;
                SET @platform = platform;
                SET @device_id = device_id;
                
                INSERT INTO `esp_update_log`(`date`, `device_id`, `platform`, `fparam1`, `fparam2`, `fparam3`, `fparam4`) VALUES (NOW(),device_id,platform,fparam1,fparam2,fparam3,fparam4);
                
                SELECT u.`protocols`, u.`host`, u.`port`, u.`path` INTO @protocols, @host, @port, @path FROM supla_iodevice AS d, esp_update AS u WHERE d.id = @device_id AND u.`platform` = @platform AND u.`fparam1` = @fparam1 AND u.`fparam2` = @fparam2 AND u.`device_name` = d.name AND u.`latest_software_version` != d.`software_version` AND ( u.`device_id` = 0 OR u.`device_id` = device_id ) LIMIT 1;
                
                SET protocols = @protocols;
                SET host = @host;
                SET port = @port;
                SET path = @path;
            END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_oauth_add_client_for_app` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_oauth_add_client_for_app`(IN `_random_id` VARCHAR(255) CHARSET utf8, 
IN `_secret` VARCHAR(255) CHARSET utf8, OUT `_id` INT(11))
    NO SQL
BEGIN

SET @lck = 0;
SET @id_exists = 0;

SELECT GET_LOCK('oauth_add_client', 2) INTO @lck;

IF @lck = 1 THEN

 SELECT id INTO @id_exists FROM `supla_oauth_clients` WHERE `type` = 2 LIMIT 1;

  IF @id_exists <> 0 THEN
     SELECT @id_exists INTO _id;
  ELSE 
     INSERT INTO `supla_oauth_clients`(
         `random_id`, `redirect_uris`, 
         `secret`, `allowed_grant_types`, `type`) VALUES 
     (_random_id, 'a:0:{}', _secret,'a:2:{i:0;s:8:"password";i:1;s:13:"refresh_token";}',2);
     
     SELECT LAST_INSERT_ID() INTO _id;
     SELECT RELEASE_LOCK('oauth_add_client') INTO @lck;
  END IF;

END IF;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_oauth_add_token_for_app` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_oauth_add_token_for_app`(IN `_user_id` INT(11), IN `_token` VARCHAR(255) CHARSET utf8, 
IN `_expires_at` INT(11), IN `_access_id` INT(11), OUT `_id` INT(11))
    NO SQL
BEGIN

SET @client_id = 0;

SELECT `id` INTO @client_id FROM `supla_oauth_clients` WHERE `type` = 2 LIMIT 1;

IF @client_id <> 0 AND EXISTS(SELECT 1 FROM `supla_accessid` WHERE `user_id` = _user_id AND `id` = _access_id) THEN 

  INSERT INTO `supla_oauth_access_tokens`(`client_id`, `user_id`, `token`, `expires_at`, `scope`, `access_id`) VALUES 
   (@client_id, _user_id, _token, _expires_at, 'channels_r channels_files', _access_id);
  SELECT LAST_INSERT_ID() INTO _id;

END IF;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_on_channeladded` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_on_channeladded`(IN `_device_id` INT, IN `_channel_id` INT)
    NO SQL
BEGIN
                SET @type = NULL;
                SELECT type INTO @type FROM supla_dev_channel WHERE `func` = 0 AND id = _channel_id;
                IF @type = 3000 THEN
                    UPDATE supla_dev_channel SET `func` = 40 WHERE id = _channel_id;
                END IF;
            END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_on_newclient` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_on_newclient`(IN `_client_id` INT)
    NO SQL
BEGIN	
			END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_on_newdevice` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_on_newdevice`(IN `_device_id` INT)
    MODIFIES SQL DATA
BEGIN
            END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_channel_caption` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_channel_caption`(IN `_user_id` INT, IN `_channel_id` INT, IN `_caption` VARCHAR(100) CHARSET utf8mb4)
    NO SQL
UPDATE supla_dev_channel SET caption = _caption WHERE id = _channel_id AND user_id = _user_id ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_channel_function` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_channel_function`(IN `_user_id` INT, IN `_channel_id` INT, IN `_func` INT)
    NO SQL
UPDATE supla_dev_channel SET func = _func WHERE id = _channel_id AND user_id = _user_id AND type = 8000 ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_registration_enabled` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_registration_enabled`(IN `user_id` INT, IN `iodevice_sec` INT, IN `client_sec` INT)
    NO SQL
BEGIN IF iodevice_sec >= 0 THEN SET @date = NULL; IF iodevice_sec > 0 THEN SET @date = DATE_ADD(UTC_TIMESTAMP, INTERVAL iodevice_sec SECOND); END IF; UPDATE supla_user SET iodevice_reg_enabled = @date WHERE id = user_id; END IF; IF client_sec >= 0 THEN SET @date = NULL; IF client_sec > 0 THEN SET @date = DATE_ADD(UTC_TIMESTAMP, INTERVAL client_sec SECOND); END IF; UPDATE supla_user SET client_reg_enabled = @date WHERE id = user_id; END IF; END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_amazon_alexa` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_amazon_alexa`(IN `_access_token` VARCHAR(1024) CHARSET utf8, IN `_refresh_token` VARCHAR(1024) CHARSET utf8, IN `_expires_in` INT, IN `_user_id` INT)
    NO SQL
BEGIN UPDATE supla_amazon_alexa SET `access_token` = _access_token, `refresh_token` = _refresh_token, `expires_at` = DATE_ADD(UTC_TIMESTAMP(), INTERVAL _expires_in second) WHERE `user_id` = _user_id; END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_channel_value` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_channel_value`(
    IN `_id` INT,
    IN `_user_id` INT,
    IN `_value` VARBINARY(8),
    IN `_validity_time_sec` INT
)
    NO SQL
BEGIN
    IF _validity_time_sec > 0 THEN
        SET @valid_to = DATE_ADD(UTC_TIMESTAMP(), INTERVAL _validity_time_sec SECOND);
        
        INSERT INTO `supla_dev_channel_value` (`channel_id`, `user_id`, `update_time`, `valid_to`, `value`) 
          VALUES(_id, _user_id, UTC_TIMESTAMP(), @valid_to, _value)
        ON DUPLICATE KEY UPDATE `value` = _value, update_time = UTC_TIMESTAMP(), valid_to = @valid_to;
         
    ELSE
        DELETE FROM `supla_dev_channel_value` WHERE `channel_id` = _id;
    END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_client` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_client`(IN `_access_id` INT(11), IN `_name` VARCHAR(100) CHARSET utf8mb4, IN `_last_ipv4` INT(10) UNSIGNED, IN `_software_version` VARCHAR(20) CHARSET utf8, IN `_protocol_version` INT(11), IN `_auth_key` VARCHAR(64) CHARSET utf8, IN `_id` INT(11))
    NO SQL
BEGIN

UPDATE `supla_client`

SET
`access_id` = _access_id,
`name` = _name,
`last_access_date` = UTC_TIMESTAMP(),
`last_access_ipv4` = _last_ipv4,
`software_version` = _software_version,
`protocol_version` = _protocol_version WHERE `id` = _id;

IF _auth_key IS NOT NULL THEN
  UPDATE `supla_client`
  SET `auth_key` = _auth_key WHERE `id` = _id AND `auth_key` IS NULL;
END IF;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_google_home` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_google_home`(IN `_access_token` VARCHAR(255) CHARSET utf8, IN `_user_id` INT)
    NO SQL
BEGIN UPDATE supla_google_home SET `access_token` = _access_token WHERE `user_id` = _user_id; END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_iodevice` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_iodevice`(IN `_name` VARCHAR(100) CHARSET utf8mb4, IN `_last_ipv4` INT(10) UNSIGNED, IN `_software_version` VARCHAR(20) CHARSET utf8, IN `_protocol_version` INT(11), IN `_original_location_id` INT(11), IN `_auth_key` VARCHAR(64) CHARSET utf8, IN `_id` INT(11))
    NO SQL
BEGIN
UPDATE `supla_iodevice`
SET
`name` = _name,
`last_connected` = UTC_TIMESTAMP(),
`last_ipv4` = _last_ipv4,
`software_version` = _software_version,
`protocol_version` = _protocol_version,
original_location_id = _original_location_id WHERE `id` = _id;
IF _auth_key IS NOT NULL THEN
  UPDATE `supla_iodevice`
  SET `auth_key` = _auth_key WHERE `id` = _id AND `auth_key` IS NULL;
END IF;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_state_webhook` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_state_webhook`(IN `_access_token` VARCHAR(255) CHARSET utf8, IN `_refresh_token` VARCHAR(255) CHARSET utf8, IN `_expires_in` INT, IN `_user_id` INT)
    NO SQL
BEGIN UPDATE supla_state_webhooks SET `access_token` = _access_token, `refresh_token` = _refresh_token, `expires_at` = DATE_ADD(UTC_TIMESTAMP(), INTERVAL _expires_in second) WHERE `user_id` = _user_id; END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;

--
-- Final view structure for view `supla_v_client`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_client`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_client`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_client` AS select `c`.`id` AS `id`,`c`.`access_id` AS `access_id`,`c`.`guid` AS `guid`,`c`.`name` AS `name`,`c`.`reg_ipv4` AS `reg_ipv4`,`c`.`reg_date` AS `reg_date`,`c`.`last_access_ipv4` AS `last_access_ipv4`,`c`.`last_access_date` AS `last_access_date`,`c`.`software_version` AS `software_version`,`c`.`protocol_version` AS `protocol_version`,`c`.`enabled` AS `enabled`,`a`.`user_id` AS `user_id` from (`supla_client` `c` join `supla_accessid` `a` on(`a`.`id` = `c`.`access_id`)) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_client_channel`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_client_channel`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_channel`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_client_channel` AS select `c`.`id` AS `id`,`c`.`type` AS `type`,`c`.`func` AS `func`,ifnull(`c`.`param1`,0) AS `param1`,ifnull(`c`.`param2`,0) AS `param2`,`c`.`caption` AS `caption`,ifnull(`c`.`param3`,0) AS `param3`,`c`.`text_param1` AS `text_param1`,`c`.`text_param2` AS `text_param2`,`c`.`text_param3` AS `text_param3`,ifnull(`d`.`manufacturer_id`,0) AS `manufacturer_id`,ifnull(`d`.`product_id`,0) AS `product_id`,ifnull(`c`.`user_icon_id`,0) AS `user_icon_id`,`c`.`user_id` AS `user_id`,`c`.`channel_number` AS `channel_number`,`c`.`iodevice_id` AS `iodevice_id`,`cl`.`id` AS `client_id`,case ifnull(`c`.`location_id`,0) when 0 then `d`.`location_id` else `c`.`location_id` end AS `location_id`,ifnull(`c`.`alt_icon`,0) AS `alt_icon`,`d`.`protocol_version` AS `protocol_version`,ifnull(`c`.`flags`,0) AS `flags`,`v`.`value` AS `value`,time_to_sec(timediff(`v`.`valid_to`,utc_timestamp())) AS `validity_time_sec` from ((((((`supla_dev_channel` `c` join `supla_iodevice` `d` on(`d`.`id` = `c`.`iodevice_id`)) join `supla_location` `l` on(`l`.`id` = (case ifnull(`c`.`location_id`,0) when 0 then `d`.`location_id` else `c`.`location_id` end))) join `supla_rel_aidloc` `r` on(`r`.`location_id` = `l`.`id`)) join `supla_accessid` `a` on(`a`.`id` = `r`.`access_id`)) join `supla_client` `cl` on(`cl`.`access_id` = `r`.`access_id`)) left join `supla_dev_channel_value` `v` on(`c`.`id` = `v`.`channel_id` and `v`.`valid_to` >= utc_timestamp())) where (`c`.`func` is not null and `c`.`func` <> 0 or `c`.`type` = 8000) and ifnull(`c`.`hidden`,0) = 0 and `d`.`enabled` = 1 and `l`.`enabled` = 1 and `a`.`enabled` = 1 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_client_channel_group`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_client_channel_group`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_channel_group`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_client_channel_group` AS select `g`.`id` AS `id`,`g`.`func` AS `func`,`g`.`caption` AS `caption`,`g`.`user_id` AS `user_id`,`g`.`location_id` AS `location_id`,ifnull(`g`.`alt_icon`,0) AS `alt_icon`,ifnull(`g`.`user_icon_id`,0) AS `user_icon_id`,`cl`.`id` AS `client_id` from ((((`supla_dev_channel_group` `g` join `supla_location` `l` on(`l`.`id` = `g`.`location_id`)) join `supla_rel_aidloc` `r` on(`r`.`location_id` = `l`.`id`)) join `supla_accessid` `a` on(`a`.`id` = `r`.`access_id`)) join `supla_client` `cl` on(`cl`.`access_id` = `r`.`access_id`)) where `g`.`func` is not null and ifnull(`g`.`hidden`,0) = 0 and `g`.`func` <> 0 and `l`.`enabled` = 1 and `a`.`enabled` = 1 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_client_location`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_client_location`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_client_location`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_client_location` AS select `l`.`id` AS `id`,`l`.`caption` AS `caption`,`c`.`id` AS `client_id` from ((`supla_rel_aidloc` `al` join `supla_location` `l` on(`l`.`id` = `al`.`location_id`)) join `supla_client` `c` on(`c`.`access_id` = `al`.`access_id`)) where `l`.`enabled` = 1 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_device_accessid`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_device_accessid`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_device_accessid`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_device_accessid` AS select `a`.`id` AS `id`,`a`.`user_id` AS `user_id`,cast(`a`.`enabled` as unsigned) AS `enabled`,`a`.`password` AS `password`,`u`.`limit_client` AS `limit_client` from (`supla_accessid` `a` join `supla_user` `u` on(`u`.`id` = `a`.`user_id`)) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_device_location`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_device_location`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_device_location`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_device_location` AS select `l`.`id` AS `id`,`l`.`user_id` AS `user_id`,cast(`l`.`enabled` as unsigned) AS `enabled`,`u`.`limit_iodev` AS `limit_iodev`,`l`.`password` AS `password` from (`supla_location` `l` join `supla_user` `u` on(`u`.`id` = `l`.`user_id`)) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_rel_cg`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_rel_cg`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_rel_cg`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_rel_cg` AS select `r`.`group_id` AS `group_id`,`r`.`channel_id` AS `channel_id`,`c`.`iodevice_id` AS `iodevice_id`,`d`.`protocol_version` AS `protocol_version`,`g`.`client_id` AS `client_id`,`c`.`hidden` AS `channel_hidden` from (((`supla_v_client_channel_group` `g` join `supla_rel_cg` `r` on(`r`.`group_id` = `g`.`id`)) join `supla_dev_channel` `c` on(`c`.`id` = `r`.`channel_id`)) join `supla_iodevice` `d` on(`d`.`id` = `c`.`iodevice_id`)) where `d`.`enabled` = 1 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_user_channel_group`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_user_channel_group`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_user_channel_group`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_user_channel_group` AS select `g`.`id` AS `id`,`g`.`func` AS `func`,`g`.`caption` AS `caption`,`g`.`user_id` AS `user_id`,`g`.`location_id` AS `location_id`,ifnull(`g`.`alt_icon`,0) AS `alt_icon`,`rel`.`channel_id` AS `channel_id`,`c`.`iodevice_id` AS `iodevice_id`,ifnull(`g`.`hidden`,0) AS `hidden` from ((((`supla_dev_channel_group` `g` join `supla_location` `l` on(`l`.`id` = `g`.`location_id`)) join `supla_rel_cg` `rel` on(`rel`.`group_id` = `g`.`id`)) join `supla_dev_channel` `c` on(`c`.`id` = `rel`.`channel_id`)) join `supla_iodevice` `d` on(`d`.`id` = `c`.`iodevice_id`)) where `g`.`func` is not null and `g`.`func` <> 0 and `l`.`enabled` = 1 and `d`.`enabled` = 1 */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2020-09-18 17:04:19
