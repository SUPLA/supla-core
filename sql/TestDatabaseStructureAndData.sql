-- MySQL dump 10.19  Distrib 10.3.34-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: supla_test
-- ------------------------------------------------------
-- Server version	10.3.34-MariaDB-0+deb10u1-log

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
  `fparam3` int(11) NOT NULL DEFAULT 0,
  `fparam4` int(11) NOT NULL DEFAULT 0,
  `protocols` tinyint(4) NOT NULL,
  `host` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `port` int(11) NOT NULL,
  `path` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `is_synced` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `device_name` (`device_name`),
  KEY `latest_software_version` (`latest_software_version`),
  KEY `fparam1` (`fparam1`),
  KEY `fparam2` (`fparam2`),
  KEY `platform` (`platform`),
  KEY `device_id` (`device_id`),
  KEY `fparam3` (`fparam3`),
  KEY `fparam4` (`fparam4`)
) ENGINE=InnoDB AUTO_INCREMENT=153 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `esp_update`
--

LOCK TABLES `esp_update` WRITE;
/*!40000 ALTER TABLE `esp_update` DISABLE KEYS */;
INSERT INTO `esp_update` VALUES (77,0,'SUPLA-GATE-MODULE',1,'2.0.1',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=gate_module2_wroom_user2.2048.new.5.bin',1),(78,0,'SUPLA-GATE-MODULE',1,'2.0.1',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=gate_module2_wroom_user1.2048.new.5.bin',1),(79,0,'NICE Wi-Fi',1,'2.7.28',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_sbw_01_2728_user2.2048_DIO.new.5.bin',1),(80,0,'NICE Wi-Fi',1,'2.7.28',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_sbw_01_2728_user1.2048_DIO.new.5.bin',1),(81,0,'NICE Wi-Fi SBP-01',1,'2.7.28',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_sbw_01_2728_user2.2048_DIO.new.5.bin',1),(82,0,'NICE Wi-Fi SBP-01',1,'2.7.28',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_sbw_01_2728_user1.2048_DIO.new.5.bin',1),(83,0,'NICE Wi-Fi SCREEN',1,'2.8.47',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_srw_01_2847_user2.2048_DIO.new.5.sdk3x.bin',1),(84,0,'NICE Wi-Fi SCREEN',1,'2.8.47',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=n_srw_01_2847_user1.2048_DIO.new.5.sdk3x.bin',1),(85,0,'ZAMEL SRW-03',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_03_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(86,0,'ZAMEL SRW-03',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_03_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(87,0,'ZAMEL ROW-02 FIX',1,'9.9.9',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_01to02_user2.2048_DIO.new.5.bin',1),(88,0,'ZAMEL ROW-02 FIX',1,'9.9.9',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_01to02_user1.2048_DIO.new.5.bin',1),(89,0,'PEVEKO VALVE',1,'2.7.21',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=peveko_valve_user2.2048_DIO.new.5.bin',1),(90,0,'PEVEKO VALVE',1,'2.7.21',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=peveko_valve_user1.2048_DIO.new.5.bin',1),(91,0,'VARILIGHT V-PRO SMART',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=vl_dimmer_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(92,0,'VARILIGHT V-PRO SMART',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=vl_dimmer_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(93,0,'ZAMEL SRW-01',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(94,0,'ZAMEL SRW-01',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(95,0,'ZAMEL SBW-01',1,'2.7.24',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_sbw_01_2724_user2.2048_DIO.new.5.bin',1),(96,0,'ZAMEL SBW-01',1,'2.7.24',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_sbw_01_2724_user1.2048_DIO.new.5.bin',1),(97,0,'ZAMEL SBW-02',1,'2.7.24',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_sbw_02_2724_user2.2048_DIO.new.5.bin',1),(98,0,'ZAMEL SBW-02',1,'2.7.24',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_sbw_02_2724_user1.2048_DIO.new.5.bin',1),(99,0,'ZAMEL ROW-02',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(100,0,'ZAMEL ROW-02',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(101,0,'ZAMEL ROW-01',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_01_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(102,0,'ZAMEL ROW-01',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_01_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(103,0,'ZAMEL LIW-01',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_liw_01_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(104,0,'ZAMEL LIW-01',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_liw_01_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(105,0,'ZAMEL SLW-01',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_slw_01_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(106,0,'ZAMEL SLW-01',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_slw_01_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(107,0,'ZAMEL SLW-02',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_slw_02_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(108,0,'ZAMEL SLW-02',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_slw_02_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(109,0,'ZAMEL ROW-01/16A',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_16A_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(110,0,'ZAMEL ROW-01/16A',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_16A_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(111,0,'ZAMEL ROW-04',1,'2.7.25',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_04m_2725_user2.2048_DIO.new.5.bin',1),(112,0,'ZAMEL ROW-04',1,'2.7.25',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_04m_2725_user1.2048_DIO.new.5.bin',1),(113,0,'ZAMEL ROW-04m',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_04m_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(114,0,'ZAMEL ROW-04m',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_04m_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(115,0,'UV TRONIC',1,'2.7.25',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=uvtronic_2725_user2.2048_DIO.new.5.bin',1),(116,0,'UV TRONIC',1,'2.7.25',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=uvtronic_2725_user1.2048_DIO.new.5.bin',1),(117,0,'ZAMEL MEW-01',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_mew_01_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(118,0,'ZAMEL MEW-01',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_mew_01_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(119,0,'HEATPOL HOME+',1,'2.7.26',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=hp_homeplus_2726_user2.2048_DIO.new.5.bin',1),(120,0,'HEATPOL HOME+',1,'2.7.26',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=hp_homeplus_2726_user1.2048_DIO.new.5.bin',1),(121,0,'ZAMEL SRW-02m',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_02m_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(122,0,'ZAMEL SRW-02m',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_02m_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(123,0,'ZAMEL SRW-02',1,'2.7.25',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_02m_2725_user2.2048_DIO.new.5.bin',1),(124,0,'ZAMEL SRW-02',1,'2.7.25',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_02m_2725_user1.2048_DIO.new.5.bin',1),(125,0,'ZAMEL PNW-01',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_pnw_01_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(126,0,'ZAMEL PNW-01',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_pnw_01_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(127,0,'ZAMEL DIW-01',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_diw_01_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(128,0,'ZAMEL DIW-01',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_diw_01_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(129,0,'ZAMEL LEW-01',1,'2.8.7',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_lew_01_287_user2.2048_DIO.new.5.sdk3x.bin',1),(130,0,'ZAMEL LEW-01',1,'2.8.7',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_lew_01_287_user1.2048_DIO.new.5.sdk3x.bin',1),(131,0,'ZAMEL mROW-02',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_mini_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(132,0,'ZAMEL mROW-02',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_mini_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(133,0,'ZAMEL ROW-02m',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02m_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(134,0,'ZAMEL ROW-02m',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02m_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(135,0,'Comelit WREL205',1,'2.8.46',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_mini_comelit_2846_user2.2048_DIO.new.5.sdk3x.bin',1),(136,0,'Comelit WREL205',1,'2.8.46',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_02_mini_comelit_2846_user1.2048_DIO.new.5.sdk3x.bin',1),(137,0,'Comelit WDIM100',1,'2.8.36',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_diw_01_comelit_2836_user2.2048_DIO.new.5.sdk3x.bin',1),(138,0,'Comelit WDIM100',1,'2.8.36',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_diw_01_comelit_2836_user1.2048_DIO.new.5.sdk3x.bin',1),(139,0,'Comelit WMIS300',1,'2.8.29',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_mew_01_comelit_2829_user2.2048_DIO.new.5.sdk3x.bin',1),(140,0,'Comelit WMIS300',1,'2.8.29',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_mew_01_comelit_2829_user1.2048_DIO.new.5.sdk3x.bin',1),(141,0,'Comelit WREL116',1,'2.8.47',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_16A_comelit_2847_user2.2048_DIO.new.5.sdk3x.bin',1),(142,0,'Comelit WREL116',1,'2.8.47',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_16A_comelit_2847_user1.2048_DIO.new.5.sdk3x.bin',1),(143,0,'ZAMEL mSRW-01',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_mini_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(144,0,'ZAMEL mSRW-01',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_mini_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(145,0,'Comelit WTAP100',1,'2.8.46',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_mini_comelit_2846_user2.2048_DIO.new.5.sdk3x.bin',1),(146,0,'Comelit WTAP100',1,'2.8.46',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_srw_01_mini_comelit_2846_user1.2048_DIO.new.5.sdk3x.bin',1),(147,0,'ZAMEL ROW-07',1,'2.8.48',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_07_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(148,0,'ZAMEL ROW-07',1,'2.8.48',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_row_07_2848_user1.2048_DIO.new.5.sdk3x.bin',1),(149,0,'ZAMEL OSW-01',1,'2.8.50',5,0,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_osw_01_2850_user2.2048_DIO.new.5.sdk3x.bin',1),(150,0,'ZAMEL OSW-01',1,'2.8.50',5,1,0,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=zam_osw_01_2850_user1.2048_DIO.new.5.sdk3x.bin',1),(151,0,'VARILIGHT V-PRO SMART',1,'2.8.48',5,0,1,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=vl_dimmer_pic13_2848_user2.2048_DIO.new.5.sdk3x.bin',1),(152,0,'VARILIGHT V-PRO SMART',1,'2.8.48',5,1,1,0,1,'www.acsoftware.pl',80,'support/get_esp_firmware.php?file=vl_dimmer_pic13_2848_user1.2048_DIO.new.5.sdk3x.bin',1);
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
  `version` varchar(14) COLLATE utf8mb4_unicode_ci NOT NULL,
  `executed_at` datetime NOT NULL COMMENT '(DC2Type:datetime_immutable)',
  PRIMARY KEY (`version`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `migration_versions`
--

LOCK TABLES `migration_versions` WRITE;
/*!40000 ALTER TABLE `migration_versions` DISABLE KEYS */;
INSERT INTO `migration_versions` VALUES ('20170101000000','2022-05-04 19:26:18'),('20170414101854','2022-05-04 19:26:19'),('20170612204116','2022-05-04 19:26:19'),('20170818114139','2022-05-04 19:26:20'),('20171013140904','2022-05-04 19:26:21'),('20171208222022','2022-05-04 19:26:21'),('20171210105120','2022-05-04 19:26:21'),('20180108224520','2022-05-04 19:26:21'),('20180113234138','2022-05-04 19:26:21'),('20180116184415','2022-05-04 19:26:21'),('20180203231115','2022-05-04 19:26:21'),('20180208145738','2022-05-04 19:26:21'),('20180224184251','2022-05-04 19:26:21'),('20180324222844','2022-05-04 19:26:21'),('20180326134725','2022-05-04 19:26:21'),('20180403175932','2022-05-04 19:26:21'),('20180403203101','2022-05-04 19:26:21'),('20180403211558','2022-05-04 19:26:21'),('20180411202101','2022-05-04 19:26:21'),('20180411203913','2022-05-04 19:26:21'),('20180416201401','2022-05-04 19:26:21'),('20180423121539','2022-05-04 19:26:21'),('20180507095139','2022-05-04 19:26:21'),('20180518131234','2022-05-04 19:26:22'),('20180707221458','2022-05-04 19:26:22'),('20180717094843','2022-05-04 19:26:22'),('20180723132652','2022-05-04 19:26:22'),('20180807083217','2022-05-04 19:26:22'),('20180812205513','2022-05-04 19:26:22'),('20180814155501','2022-05-04 19:26:22'),('20180914222230','2022-05-04 19:26:23'),('20181001221229','2022-05-04 19:26:23'),('20181007112610','2022-05-04 19:26:23'),('20181019115859','2022-05-04 19:26:23'),('20181024164957','2022-05-04 19:26:24'),('20181025171850','2022-05-04 19:26:24'),('20181026171557','2022-05-04 19:26:24'),('20181105144611','2022-05-04 19:26:24'),('20181126225634','2022-05-04 19:26:24'),('20181129170610','2022-05-04 19:26:24'),('20181129195431','2022-05-04 19:26:25'),('20181129231132','2022-05-04 19:26:25'),('20181204174603','2022-05-04 19:26:25'),('20181205092324','2022-05-04 19:26:25'),('20181222001450','2022-05-04 19:26:25'),('20190105130410','2022-05-04 19:26:25'),('20190117075805','2022-05-04 19:26:25'),('20190219184847','2022-05-04 19:26:25'),('20190325215115','2022-05-04 19:26:25'),('20190401151822','2022-05-04 19:26:25'),('20190720215803','2022-05-04 19:26:25'),('20190813232026','2022-05-04 19:26:25'),('20190815154016','2022-05-04 19:26:26'),('20191226160845','2022-05-04 19:26:26'),('20200108201101','2022-05-04 19:26:26'),('20200123235701','2022-05-04 19:26:26'),('20200124084227','2022-05-04 19:26:26'),('20200204170901','2022-05-04 19:26:26'),('20200210145902','2022-05-04 19:26:26'),('20200229122103','2022-05-04 19:26:26'),('20200322123636','2022-05-04 19:26:26'),('20200412183701','2022-05-04 19:26:26'),('20200414213205','2022-05-04 19:26:26'),('20200416225304','2022-05-04 19:26:26'),('20200419190150','2022-05-04 19:26:26'),('20200430113342','2022-05-04 19:26:27'),('20200514132030','2022-05-04 19:26:27'),('20200515102311','2022-05-04 19:26:27'),('20200518171230','2022-05-04 19:26:27'),('20200724155001','2022-05-04 19:26:27'),('20200807131101','2022-05-04 19:26:27'),('20200811141801','2022-05-04 19:26:27'),('20200813113801','2022-05-04 19:26:27'),('20200813133501','2022-05-04 19:26:27'),('20200911231401','2022-05-04 19:26:27'),('20201113112233','2022-05-04 19:26:27'),('20201213133718','2022-05-04 19:26:27'),('20201214102230','2022-05-04 19:26:27'),('20210105164727','2022-05-04 19:26:27'),('20210118124714','2022-05-04 19:26:27'),('20210228201414','2022-05-04 19:26:27'),('20210323095216','2022-05-04 19:26:27'),('20210419201821','2022-05-04 19:26:27'),('20210525104812','2022-05-04 19:26:27'),('20210915221319','2022-05-04 19:26:28'),('20210917203710','2022-05-04 19:26:28'),('20211005074509','2022-05-04 19:26:28'),('20211108120835','2022-05-04 19:26:28'),('20211123193415','2022-05-04 19:26:28'),('20211205215406','2022-05-04 19:26:28'),('20211218174444','2022-05-04 19:26:28'),('20220208164512','2022-05-04 19:26:28'),('20220222110707','2022-05-04 19:57:43'),('20220309061811','2022-05-04 19:57:43'),('20220309061812','2022-05-04 19:57:43'),('20220404100406','2022-07-19 21:40:53'),('20220718203129','2022-07-19 21:40:53'),('20220719210858','2022-07-19 21:40:53'),('20220929090847','2022-10-23 10:37:57'),('20221005003914','2022-10-23 10:37:57'),('20221010103958','2022-10-23 10:37:58'),('20221020225729','2022-10-23 10:53:42');
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
  `active_from` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `active_to` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `active_hours` varchar(768) COLLATE utf8_unicode_ci DEFAULT NULL,
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
INSERT INTO `supla_accessid` VALUES (2,2,'3311dbb5','Access Identifier #1',1,NULL,NULL,NULL);
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
) ENGINE=InnoDB AUTO_INCREMENT=31 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_audit`
--

LOCK TABLES `supla_audit` WRITE;
/*!40000 ALTER TABLE `supla_audit` DISABLE KEYS */;
INSERT INTO `supla_audit` VALUES (1,NULL,2,'2020-04-16 08:55:27',2457392666,'',0),(2,NULL,2,'2020-04-16 08:55:32',2457392666,'',0),(3,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(4,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(5,NULL,2,'2020-04-16 08:55:40',2457392666,'',0),(6,NULL,2,'2020-04-16 08:55:46',2457392666,'',0),(7,NULL,2,'2020-04-16 14:05:47',2457392666,'test@supla.org',2),(8,NULL,7,'2020-04-16 14:06:28',NULL,'test@supla.org',1),(9,2,1,'2020-04-16 14:13:12',2457392666,'test@supla.org',NULL),(10,NULL,2,'2020-04-16 14:24:06',2457392666,'',0),(11,NULL,2,'2020-04-16 14:24:07',2457392666,'',0),(12,2,2,'2020-04-16 20:08:42',2457392666,'test@supla.org',2),(13,2,2,'2020-04-16 20:08:49',2457392666,'test@supla.org',2),(14,2,1,'2020-04-16 20:09:18',2457392666,'test@supla.org',NULL),(15,NULL,2,'2020-05-19 20:55:27',2457392666,'',0),(16,NULL,2,'2020-05-19 20:55:27',2457392666,'',0),(17,NULL,2,'2020-05-19 20:55:28',2457392666,'',0),(18,2,1,'2022-05-04 19:30:21',2457392666,'test@supla.org',NULL),(19,2,1,'2022-05-07 11:50:42',2457392666,'test@supla.org',NULL),(20,NULL,2,'2022-05-07 15:10:43',2457392666,'',0),(21,NULL,2,'2022-05-07 15:10:50',2457392666,'',0),(22,NULL,2,'2022-05-07 15:10:57',2457392666,'',0),(23,NULL,2,'2022-05-07 15:11:04',2457392666,'',0),(24,NULL,2,'2022-05-07 15:11:11',2457392666,'',0),(25,NULL,2,'2022-05-07 15:11:18',2457392666,'',0),(26,NULL,2,'2022-05-07 15:11:25',2457392666,'',0),(27,2,2,'2022-05-07 15:11:35',2457392666,'test@supla.org',2),(28,2,1,'2022-05-07 15:11:44',2457392666,'test@supla.org',NULL),(29,NULL,2,'2022-07-19 21:40:58',2457392666,'',0),(30,NULL,2,'2022-07-19 21:41:00',2457392666,'',0);
/*!40000 ALTER TABLE `supla_audit` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_auto_gate_closing`
--

DROP TABLE IF EXISTS `supla_auto_gate_closing`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_auto_gate_closing` (
  `channel_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT 0,
  `active_from` date DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `active_to` date DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `active_hours` varchar(768) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT NULL,
  `max_time_open` int(11) NOT NULL,
  `seconds_open` int(11) DEFAULT NULL,
  `closing_attempt` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `last_seen_open` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  PRIMARY KEY (`channel_id`),
  KEY `IDX_E176CB9FA76ED395` (`user_id`),
  CONSTRAINT `FK_E176CB9F72F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_E176CB9FA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_auto_gate_closing`
--

LOCK TABLES `supla_auto_gate_closing` WRITE;
/*!40000 ALTER TABLE `supla_auto_gate_closing` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_auto_gate_closing` ENABLE KEYS */;
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
  `user_config` varchar(2048) COLLATE utf8_unicode_ci DEFAULT NULL,
  `param4` int(11) NOT NULL DEFAULT 0,
  `properties` varchar(2048) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQUE_CHANNEL` (`iodevice_id`,`channel_number`),
  KEY `IDX_81E928C9125F95D6` (`iodevice_id`),
  KEY `IDX_81E928C9A76ED395` (`user_id`),
  KEY `IDX_81E928C964D218E` (`location_id`),
  KEY `IDX_81E928C9CB4C938` (`user_icon_id`),
  KEY `supla_dev_channel_param1_idx` (`param1`),
  KEY `supla_dev_channel_param2_idx` (`param2`),
  KEY `supla_dev_channel_param3_idx` (`param3`),
  KEY `supla_dev_channel_param4_idx` (`param4`),
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
INSERT INTO `supla_dev_channel` VALUES (2,2,2,0,NULL,6010,410,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(140,73,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(141,73,2,1,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(142,73,2,2,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(143,73,2,3,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(144,74,2,0,'',5000,310,0,0,5600,0,'PLN',NULL,NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0.56,\"currency\":\"PLN\"}',0,NULL),(146,76,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(157,83,2,0,'',3000,40,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"temperatureAdjustment\":0}',0,NULL),(158,83,2,1,'',3020,45,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"humidityAdjustment\":0,\"temperatureAdjustment\":0}',0,NULL),(159,83,2,2,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(160,83,2,3,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(161,83,2,4,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(162,83,2,5,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(163,83,2,6,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(164,83,2,7,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(165,83,2,8,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(166,83,2,9,NULL,1000,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(167,83,2,10,'Szlaban',2900,20,239,500,0,0,NULL,NULL,NULL,2,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}',0,NULL),(168,83,2,11,'',2900,90,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}',0,NULL),(169,83,2,12,NULL,4010,0,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'[]',0,NULL),(170,83,2,13,'',4000,180,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(171,83,2,14,'',4020,200,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(172,83,2,15,'',2900,130,239,0,0,0,NULL,NULL,NULL,4,0,NULL,0,NULL,'[]',0,NULL),(173,83,2,16,'',2900,110,16,300,0,400,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingTimeS\":30,\"closingTimeS\":40,\"timeSettingAvailable\":true,\"openingSensorChannelId\":null}',0,NULL),(174,83,2,17,'',2900,20,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}',0,NULL),(175,83,2,18,'',2900,30,239,500,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'{\"openingSensorChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}',0,NULL),(176,84,2,0,'',5010,315,0,7508272,5500,1000,'PLN',NULL,NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0.55,\"impulsesPerUnit\":1000,\"currency\":\"PLN\",\"unit\":null,\"initialValue\":75082.72}',0,NULL),(177,83,2,19,'',3044,260,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(178,83,2,20,'',3050,280,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(179,83,2,21,'',3042,250,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(180,83,2,22,'',3048,270,0,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(282,135,2,0,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(283,135,2,1,NULL,2900,140,96,0,0,0,NULL,NULL,NULL,0,0,NULL,0,NULL,'[]',0,NULL),(288,84,2,1,NULL,5010,315,0,0,0,1,NULL,'impl',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"impl\",\"initialValue\":0}',0,NULL),(289,84,2,2,'Overflow counter ‚ù§Ô∏è',5010,315,0,0,0,1,NULL,'x',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"x\",\"initialValue\":0}',0,NULL),(290,84,2,3,NULL,5010,315,0,0,0,1,NULL,'sec',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"sec\",\"initialValue\":0}',0,NULL),(291,84,2,4,NULL,5010,315,0,0,0,1,NULL,'n',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"n\",\"initialValue\":0}',0,NULL),(296,84,2,5,NULL,5010,315,0,0,0,1,NULL,'Byte',NULL,0,0,NULL,0,NULL,'{\"pricePerUnit\":0,\"impulsesPerUnit\":1,\"currency\":null,\"unit\":\"Byte\",\"initialValue\":0}',0,NULL),(301,142,2,0,NULL,7000,500,0,0,0,0,NULL,NULL,NULL,NULL,0,NULL,65536,NULL,'[]',0,NULL),(303,144,2,0,NULL,8000,110,48,0,0,0,NULL,NULL,NULL,NULL,0,NULL,1,NULL,'{\"openingTimeS\":0,\"closingTimeS\":0,\"timeSettingAvailable\":true,\"openingSensorChannelId\":null}',0,NULL),(308,146,2,0,NULL,2900,20,15,500,310,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"openingSensorChannelId\":310,\"openingSensorSecondaryChannelId\":null,\"relayTimeMs\":500,\"timeSettingAvailable\":true}',0,NULL),(309,146,2,1,NULL,2900,10,15,6000,311,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"openingSensorChannelId\":311,\"relayTimeMs\":6000,\"timeSettingAvailable\":true}',0,NULL),(310,146,2,2,NULL,1000,60,0,308,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"controllingChannelId\":308,\"controllingSecondaryChannelId\":null,\"invertedLogic\":false}',0,NULL),(311,146,2,3,NULL,1000,50,0,309,0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL,'{\"controllingChannelId\":309,\"invertedLogic\":false}',0,NULL);
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
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_dev_channel_group`
--

LOCK TABLES `supla_dev_channel_group` WRITE;
/*!40000 ALTER TABLE `supla_dev_channel_group` DISABLE KEYS */;
INSERT INTO `supla_dev_channel_group` VALUES (1,2,'',140,0,2,0,NULL);
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
-- Table structure for table `supla_em_voltage_log`
--

DROP TABLE IF EXISTS `supla_em_voltage_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_em_voltage_log` (
  `channel_id` int(11) NOT NULL,
  `date` datetime NOT NULL COMMENT '(DC2Type:stringdatetime)',
  `phase_no` tinyint(4) NOT NULL COMMENT '(DC2Type:tinyint)',
  `count_total` int(11) NOT NULL,
  `count_above` int(11) NOT NULL,
  `count_below` int(11) NOT NULL,
  `sec_total` int(11) NOT NULL,
  `sec_above` int(11) NOT NULL,
  `sec_below` int(11) NOT NULL,
  `max_sec_above` int(11) NOT NULL,
  `max_sec_below` int(11) NOT NULL,
  `min_voltage` decimal(7,2) NOT NULL,
  `max_voltage` decimal(7,2) NOT NULL,
  `avg_voltage` decimal(7,2) NOT NULL,
  `measurement_time_sec` int(11) NOT NULL,
  PRIMARY KEY (`channel_id`,`date`,`phase_no`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_em_voltage_log`
--

LOCK TABLES `supla_em_voltage_log` WRITE;
/*!40000 ALTER TABLE `supla_em_voltage_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_em_voltage_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `supla_email_notifications`
--

DROP TABLE IF EXISTS `supla_email_notifications`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `supla_email_notifications` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `body` longtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `headers` longtext COLLATE utf8mb4_unicode_ci NOT NULL,
  `queue_name` varchar(190) COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` datetime NOT NULL,
  `available_at` datetime NOT NULL,
  `delivered_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `IDX_7C77A74CFB7336F0` (`queue_name`),
  KEY `IDX_7C77A74CE3BD61CE` (`available_at`),
  KEY `IDX_7C77A74C16BA31DB` (`delivered_at`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_email_notifications`
--

LOCK TABLES `supla_email_notifications` WRITE;
/*!40000 ALTER TABLE `supla_email_notifications` DISABLE KEYS */;
/*!40000 ALTER TABLE `supla_email_notifications` ENABLE KEYS */;
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
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_location`
--

LOCK TABLES `supla_location` WRITE;
/*!40000 ALTER TABLE `supla_location` DISABLE KEYS */;
INSERT INTO `supla_location` VALUES (2,2,'8aea','Location #1',1),(3,2,'4b15','Location #2',1);
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
  `api_client_authorization_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_2402564B5F37A13B` (`token`),
  KEY `IDX_2402564B19EB6921` (`client_id`),
  KEY `IDX_2402564BA76ED395` (`user_id`),
  KEY `IDX_2402564B4FEA67CF` (`access_id`),
  KEY `IDX_2402564BCA22CF77` (`api_client_authorization_id`),
  CONSTRAINT `FK_2402564B19EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_2402564B4FEA67CF` FOREIGN KEY (`access_id`) REFERENCES `supla_accessid` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_2402564BA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_2402564BCA22CF77` FOREIGN KEY (`api_client_authorization_id`) REFERENCES `supla_oauth_client_authorizations` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_oauth_access_tokens`
--

LOCK TABLES `supla_oauth_access_tokens` WRITE;
/*!40000 ALTER TABLE `supla_oauth_access_tokens` DISABLE KEYS */;
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
  `mqtt_broker_auth_password` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
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
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
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
  `api_client_authorization_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UNIQ_B809538C5F37A13B` (`token`),
  KEY `IDX_B809538C19EB6921` (`client_id`),
  KEY `IDX_B809538CA76ED395` (`user_id`),
  KEY `IDX_B809538CCA22CF77` (`api_client_authorization_id`),
  CONSTRAINT `FK_B809538C19EB6921` FOREIGN KEY (`client_id`) REFERENCES `supla_oauth_clients` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_B809538CA76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_B809538CCA22CF77` FOREIGN KEY (`api_client_authorization_id`) REFERENCES `supla_oauth_client_authorizations` (`id`) ON DELETE CASCADE
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
INSERT INTO `supla_rel_aidloc` VALUES (2,2),(3,2);
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
INSERT INTO `supla_rel_cg` VALUES (140,1),(142,1);
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
  `caption` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `enabled` tinyint(1) NOT NULL,
  `user_icon_id` int(11) DEFAULT NULL,
  `alt_icon` tinyint(3) unsigned NOT NULL DEFAULT 0 COMMENT '(DC2Type:tinyint)',
  `estimated_execution_time` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `IDX_A4825857A76ED395` (`user_id`),
  KEY `IDX_A482585764D218E` (`location_id`),
  KEY `IDX_A4825857CB4C938` (`user_icon_id`),
  CONSTRAINT `FK_A482585764D218E` FOREIGN KEY (`location_id`) REFERENCES `supla_location` (`id`),
  CONSTRAINT `FK_A4825857A76ED395` FOREIGN KEY (`user_id`) REFERENCES `supla_user` (`id`),
  CONSTRAINT `FK_A4825857CB4C938` FOREIGN KEY (`user_icon_id`) REFERENCES `supla_user_icons` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_scene`
--

LOCK TABLES `supla_scene` WRITE;
/*!40000 ALTER TABLE `supla_scene` DISABLE KEYS */;
INSERT INTO `supla_scene` VALUES (1,2,2,'Scene #1',1,NULL,0,1000),(2,2,2,'Scene #2',1,NULL,0,5000),(3,2,2,'Scene #3',1,NULL,0,0),(4,2,3,'Scene #4',1,NULL,0,0),(5,2,2,'Scene #5',1,NULL,0,0),(6,2,2,'Scene #6',1,NULL,0,5000);
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
  `user_delay_ms` int(11) NOT NULL DEFAULT 0,
  `wait_for_completion` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `IDX_64A50CF5E019BC26` (`owning_scene_id`),
  KEY `IDX_64A50CF572F5A1AA` (`channel_id`),
  KEY `IDX_64A50CF589E4AAEE` (`channel_group_id`),
  KEY `IDX_64A50CF5166053B4` (`scene_id`),
  CONSTRAINT `FK_64A50CF5166053B4` FOREIGN KEY (`scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF572F5A1AA` FOREIGN KEY (`channel_id`) REFERENCES `supla_dev_channel` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF589E4AAEE` FOREIGN KEY (`channel_group_id`) REFERENCES `supla_dev_channel_group` (`id`) ON DELETE CASCADE,
  CONSTRAINT `FK_64A50CF5E019BC26` FOREIGN KEY (`owning_scene_id`) REFERENCES `supla_scene` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=31 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `supla_scene_operation`
--

LOCK TABLES `supla_scene_operation` WRITE;
/*!40000 ALTER TABLE `supla_scene_operation` DISABLE KEYS */;
INSERT INTO `supla_scene_operation` VALUES (17,1,140,NULL,NULL,60,NULL,0,0,0),(18,1,308,NULL,NULL,10,NULL,1000,1000,0),(19,1,170,NULL,NULL,80,'{\"brightness\":10}',0,0,0),(20,2,142,NULL,NULL,60,NULL,0,0,0),(21,2,140,NULL,NULL,70,NULL,5000,5000,0),(22,3,NULL,1,NULL,60,NULL,0,0,0),(23,3,171,NULL,NULL,80,'{\"brightness\":6,\"hue\":240,\"color_brightness\":10}',0,0,0),(24,3,NULL,NULL,1,3000,NULL,0,0,0),(26,5,173,NULL,NULL,50,'{\"percentage\":15}',0,0,0),(27,5,303,NULL,NULL,51,'{\"percentage\":25}',0,0,0),(28,6,171,NULL,NULL,80,'{\"brightness\":50,\"hue\":\"white\",\"color_brightness\":5}',0,0,0),(29,6,171,NULL,NULL,80,'{\"brightness\":20,\"hue\":\"random\",\"color_brightness\":15}',5000,5000,0),(30,4,140,NULL,NULL,10100,'{\"sourceDeviceId\":73,\"sourceChannelId\":143}',0,0,0);
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
  `mode` varchar(15) COLLATE utf8_unicode_ci NOT NULL,
  `date_start` datetime NOT NULL COMMENT '(DC2Type:utcdatetime)',
  `date_end` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `enabled` tinyint(1) NOT NULL,
  `next_calculation_date` datetime DEFAULT NULL COMMENT '(DC2Type:utcdatetime)',
  `caption` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `retry` tinyint(1) NOT NULL DEFAULT 1,
  `channel_group_id` int(11) DEFAULT NULL,
  `scene_id` int(11) DEFAULT NULL,
  `config` varchar(2048) COLLATE utf8_unicode_ci DEFAULT NULL,
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
  `action` int(11) NOT NULL,
  `action_param` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
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
  `limit_aid` int(11) NOT NULL DEFAULT 10,
  `limit_loc` int(11) NOT NULL DEFAULT 10,
  `limit_iodev` int(11) NOT NULL DEFAULT 100,
  `limit_client` int(11) NOT NULL DEFAULT 200,
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
  `mqtt_broker_enabled` tinyint(1) NOT NULL DEFAULT 0,
  `mqtt_broker_auth_password` varchar(128) COLLATE utf8_unicode_ci DEFAULT NULL,
  `limit_actions_per_schedule` int(11) NOT NULL DEFAULT 20,
  `preferences` varchar(4096) COLLATE utf8_unicode_ci NOT NULL DEFAULT '{}',
  `limit_operations_per_scene` int(11) NOT NULL DEFAULT 20,
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
INSERT INTO `supla_user` VALUES (2,'6d4e39883a09d3baffe9e15b5fedc152','090874ade7b4e23aef2330e81cd4784cc3badcb0b6df34c8a0be0d91ded0b5567fde8b36bf920c5a2e1ead39aa4494d3d262a1cc4a6a93225a15fec85265858f85936af20236cb7b50a8e9a420897c966cbe22f2e55275f033b6b2b10e2b5136bcb31fcb','af52uahn388cg0kggw4008swkgc0s04','test@supla.org','$2y$13$991OYKopr/P1IPP6AvarJ.12b7edDIeuRSxz/xJKrf4K1RFrvZxwu',1,'2020-04-16 14:07:33',NULL,NULL,10,10,100,200,'Europe/Berlin',20,NULL,NULL,NULL,20,10,1,0,NULL,NULL,50,20,'pl',NULL,50,NULL,0,NULL,20,'{}',20);
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
-- Temporary table structure for view `supla_v_accessid_active`
--

DROP TABLE IF EXISTS `supla_v_accessid_active`;
/*!50001 DROP VIEW IF EXISTS `supla_v_accessid_active`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_accessid_active` (
  `id` tinyint NOT NULL,
  `user_id` tinyint NOT NULL,
  `password` tinyint NOT NULL,
  `caption` tinyint NOT NULL,
  `enabled` tinyint NOT NULL,
  `active_from` tinyint NOT NULL,
  `active_to` tinyint NOT NULL,
  `active_hours` tinyint NOT NULL,
  `is_now_active` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

--
-- Temporary table structure for view `supla_v_auto_gate_closing`
--

DROP TABLE IF EXISTS `supla_v_auto_gate_closing`;
/*!50001 DROP VIEW IF EXISTS `supla_v_auto_gate_closing`*/;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
/*!50001 CREATE TABLE `supla_v_auto_gate_closing` (
  `user_id` tinyint NOT NULL,
  `enabled` tinyint NOT NULL,
  `device_id` tinyint NOT NULL,
  `channel_id` tinyint NOT NULL,
  `is_now_active` tinyint NOT NULL,
  `max_time_open` tinyint NOT NULL,
  `seconds_open` tinyint NOT NULL,
  `closing_attempt` tinyint NOT NULL,
  `last_seen_open` tinyint NOT NULL
) ENGINE=MyISAM */;
SET character_set_client = @saved_cs_client;

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
  `param4` tinyint NOT NULL,
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
/*!50003 DROP FUNCTION IF EXISTS `supla_current_weekday_hour` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` FUNCTION `supla_current_weekday_hour`(`user_timezone` VARCHAR(50)) RETURNS varchar(3) CHARSET utf8mb4
BEGIN
            DECLARE current_weekday INT;
            DECLARE current_hour INT;
            DECLARE current_time_in_user_timezone DATETIME;
            SELECT COALESCE(CONVERT_TZ(CURRENT_TIMESTAMP, 'UTC', `user_timezone`), UTC_TIMESTAMP) INTO current_time_in_user_timezone; 
            SELECT (WEEKDAY(current_time_in_user_timezone) + 1) INTO current_weekday;
            SELECT HOUR(current_time_in_user_timezone) INTO current_hour;
            RETURN CONCAT(current_weekday, current_hour);
        END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `supla_is_now_active` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` FUNCTION `supla_is_now_active`(
    `active_from` DATETIME,
    `active_to` DATETIME,
    `active_hours` VARCHAR(768),
    `user_timezone` VARCHAR(50)
) RETURNS int(11)
BEGIN
    DECLARE
        res INT DEFAULT 1; IF `active_from` IS NOT NULL THEN
    SELECT
        (active_from <= UTC_TIMESTAMP)
    INTO res;
END IF; IF res = 1 AND `active_to` IS NOT NULL THEN
SELECT
    (active_to >= UTC_TIMESTAMP)
INTO res;
END IF; IF res = 1 AND `active_hours` IS NOT NULL THEN
SELECT
    (
        `active_hours` LIKE CONCAT(
            '%,',
            supla_current_weekday_hour(`user_timezone`),
            ',%'
        ) COLLATE utf8mb4_unicode_ci
    )
INTO res;
END IF; RETURN res;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP FUNCTION IF EXISTS `version_to_int` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` FUNCTION `version_to_int`(`version` VARCHAR(20)) RETURNS int(11)
    NO SQL
BEGIN
DECLARE result INT DEFAULT 0;
DECLARE n INT DEFAULT 0;
DECLARE m INT DEFAULT 0;
DECLARE dot_count INT DEFAULT 0;
DECLARE last_dot_pos INT DEFAULT 0;
DECLARE c VARCHAR(1);

WHILE n < LENGTH(version) DO
    SET n = n+1;
    SET c = SUBSTRING(version, n, 1);
    
    IF c <> '.' AND ( ASCII(c) < 48 OR ASCII(c) > 57 )
      THEN 
         SET result = 0;
         RETURN 0;
      END IF; 
      
    
   IF c = '.' THEN
     SET dot_count = dot_count+1;
     IF dot_count > 2 THEN 
        SET result = 0;
        RETURN 0;
     END IF;
   END IF;

   IF c = '.' OR n = LENGTH(version) THEN

      SET m = n-last_dot_pos-1;
      
      IF c <> '.' THEN
        SET m = n-last_dot_pos;
        SET dot_count = dot_count+1;
      END IF;
      
          SET result = result + POWER(10, 9-dot_count*3) * SUBSTRING(version, last_dot_pos+1, m);
      
      SET last_dot_pos = n;
   END IF;
      
END WHILE;
RETURN result;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
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
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_client`(IN `_access_id` INT(11), IN `_guid` VARBINARY(16), IN `_name` VARCHAR(100) CHARSET utf8mb4, 
IN `_reg_ipv4` INT(10) UNSIGNED, IN `_software_version` VARCHAR(20) CHARSET utf8, IN `_protocol_version` INT(11), IN `_user_id` INT(11), 
IN `_auth_key` VARCHAR(64) CHARSET utf8, OUT `_id` INT(11))
    NO SQL
BEGIN

INSERT INTO `supla_client`(`access_id`, `guid`, `name`, `enabled`, `reg_ipv4`, `reg_date`, `last_access_ipv4`, 
`last_access_date`,`software_version`, `protocol_version`, `user_id`, `auth_key`) 
VALUES (_access_id, _guid, _name, 1, _reg_ipv4, UTC_TIMESTAMP(), _reg_ipv4, UTC_TIMESTAMP(), _software_version, _protocol_version, 
_user_id, _auth_key);

SELECT LAST_INSERT_ID() INTO _id;

SELECT CONCAT('{"template": "new_client_app", "userId": ', _user_id, ', "data": {"clientAppId": ', _id, '}}') INTO @notification_data;
INSERT INTO `supla_email_notifications` (`body`, `headers`, `queue_name`, `created_at`, `available_at`)
VALUES(@notification_data, '[]', 'supla-server', NOW(), NOW());

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
/*!50003 DROP PROCEDURE IF EXISTS `supla_add_em_voltage_log_item` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_add_em_voltage_log_item`(
                IN `_channel_id` INT(11), 
                IN `_phase_no` TINYINT,
                IN `_count_total` INT(11),
                IN `_count_above` INT(11),
                IN `_count_below` INT(11),
                IN `_sec_total` INT(11),
                IN `_sec_above` INT(11),
                IN `_sec_below` INT(11),
                IN `_max_sec_above` INT(11),
                IN `_max_sec_below` INT(11),
                IN `_min_voltage` NUMERIC(7,2),
                IN `_max_voltage` NUMERIC(7,2),
                IN `_avg_voltage` NUMERIC(7,2),
                IN `_measurement_time_sec` INT(11)
            )
    NO SQL
BEGIN

            INSERT INTO `supla_em_voltage_log` (`date`,channel_id, phase_no, count_total, count_above, count_below, sec_total, sec_above, sec_below, max_sec_above, max_sec_below, min_voltage, max_voltage, avg_voltage, measurement_time_sec)
                              VALUES (UTC_TIMESTAMP(),_channel_id,_phase_no,_count_total,_count_above,_count_below,_sec_total,_sec_above,_sec_below,_max_sec_above,_max_sec_below,_min_voltage,_max_voltage,_avg_voltage,_measurement_time_sec);

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
SELECT CONCAT('{"template": "new_io_device", "userId": ', _user_id, ', "data": {"ioDeviceId": ', _id, '}}') INTO @notification_data;
INSERT INTO `supla_email_notifications` (`body`, `headers`, `queue_name`, `created_at`, `available_at`)
VALUES(@notification_data, '[]', 'supla-server', NOW(), NOW());
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
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_get_device_firmware_url`(IN `in_device_id` INT, IN `in_platform` TINYINT, IN `in_fparam1` INT, IN `in_fparam2` INT, IN `in_fparam3` INT, IN `in_fparam4` INT, OUT `out_protocols` TINYINT, OUT `out_host` VARCHAR(100), OUT `out_port` INT, OUT `out_path` VARCHAR(100))
    NO SQL
BEGIN

SET @var_protocols = 0;
SET @var_host = '';
SET @var_port = 0;
SET @var_path = '';

SET @fparam1 = in_fparam1;
SET @fparam2 = in_fparam2;
SET @fparam3 = in_fparam3;
SET @fparam4 = in_fparam4;
SET @platform = in_platform;
SET @device_id = in_device_id;

SET @update_count = 0;
SELECT COUNT(*) INTO @update_count FROM `esp_update_log` WHERE `device_id` = @device_id AND `date`  + INTERVAL 30 MINUTE >= NOW();

IF @update_count = 0 THEN

SELECT u.`protocols`, u.`host`, u.`port`, u.`path` INTO @var_protocols, @var_host, @var_port, @var_path FROM supla_iodevice AS d, esp_update AS u WHERE d.id = @device_id AND u.`platform` = @platform AND u.`fparam1` = @fparam1 AND u.`fparam2` = @fparam2 AND @fparam3 = u.`fparam3` AND @fparam4 = u.`fparam4` AND u.`device_name` = d.name AND u.`latest_software_version` != d.`software_version` AND 

(
version_to_int(d.`software_version`) = 0 OR
version_to_int(u.`latest_software_version`) = 0 OR
version_to_int(u.`latest_software_version`) > version_to_int(d.`software_version`)
)

AND ( u.`device_id` = 0 OR u.`device_id` = @device_id ) LIMIT 1;

END IF;

INSERT INTO `esp_update_log`(`date`, `device_id`, `platform`, `fparam1`, `fparam2`, `fparam3`, `fparam4`) VALUES (NOW(),in_device_id,in_platform,in_fparam1,in_fparam2,in_fparam3,in_fparam4);

SET out_protocols = @var_protocols;
SET out_host = @var_host;
SET out_port = @var_port;
SET out_path = @var_path;

END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_mark_gate_closed` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_mark_gate_closed`(IN `_channel_id` INT)
UPDATE
    `supla_auto_gate_closing`
SET
    seconds_open = NULL,
    closing_attempt = NULL,
    last_seen_open = NULL
WHERE
    channel_id = _channel_id ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_mark_gate_open` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_mark_gate_open`(IN `_channel_id` INT)
BEGIN
    -- We assume the server will mark open gates at least every minute.
    UPDATE
        `supla_auto_gate_closing`
    SET
        seconds_open = NULL,
        closing_attempt = NULL,
        last_seen_open = NULL
    WHERE
        channel_id = _channel_id AND last_seen_open IS NOT NULL AND TIMESTAMPDIFF(MINUTE, last_seen_open, UTC_TIMESTAMP()) >= 4;
        
    UPDATE
        `supla_auto_gate_closing`
    SET
        seconds_open = IFNULL(seconds_open, 0) + IFNULL(
            UNIX_TIMESTAMP(UTC_TIMESTAMP()) - UNIX_TIMESTAMP(last_seen_open),
            0),
            last_seen_open = UTC_TIMESTAMP()
        WHERE
            channel_id = _channel_id;
      
      SELECT
            max_time_open - seconds_open AS `seconds_left`
      FROM
            `supla_auto_gate_closing`
      WHERE
             channel_id = _channel_id;
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_mqtt_broker_auth` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_mqtt_broker_auth`(IN `in_suid` VARCHAR(255) CHARSET utf8, IN `in_password` VARCHAR(255) CHARSET utf8)
    NO SQL
BEGIN
                SET @hashed_password = SHA2(in_password, 512);
            SELECT 1 FROM supla_user su
            LEFT JOIN supla_oauth_client_authorizations soca ON su.id = soca.user_id
            WHERE mqtt_broker_enabled = 1 AND short_unique_id = BINARY in_suid AND(
                su.mqtt_broker_auth_password = @hashed_password OR soca.mqtt_broker_auth_password = @hashed_password
            )
            LIMIT 1;
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
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_closing_attempt` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_closing_attempt`(IN `_channel_id` INT)
UPDATE
        `supla_auto_gate_closing`
    SET
        closing_attempt = UTC_TIMESTAMP()
    WHERE
        channel_id = _channel_id ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_location_caption` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_location_caption`(IN `_user_id` INT, IN `_location_id` INT, IN `_caption` VARCHAR(100) CHARSET utf8mb4)
    NO SQL
UPDATE supla_location SET caption = _caption WHERE id = _location_id AND user_id = _user_id ;;
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
/*!50003 DROP PROCEDURE IF EXISTS `supla_set_scene_caption` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_set_scene_caption`(IN `_user_id` INT, IN `_scene_id` INT, IN `_caption` VARCHAR(255) CHARSET utf8mb4)
    NO SQL
UPDATE supla_scene SET caption = _caption WHERE id = _scene_id AND user_id = _user_id ;;
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
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_channel_flags` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_channel_flags`(IN `_channel_id` INT, IN `_user_id` INT, IN `_flags` INT)
    NO SQL
UPDATE supla_dev_channel SET flags = IFNULL(flags, 0) | IFNULL(_flags, 0) WHERE id = _channel_id AND user_id = _user_id ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_channel_params` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_channel_params`(
    IN `_id` INT,
    IN `_user_id` INT,
    IN `_param1` INT,
    IN `_param2` INT,
    IN `_param3` INT,
    IN `_param4` INT
)
    NO SQL
BEGIN
    UPDATE
        supla_dev_channel
    SET
        param1 = _param1,
        param2 = _param2,
        param3 = _param3,
        param4 = _param4
    WHERE
        id = _id AND user_id = _user_id ; 
END ;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
/*!50003 DROP PROCEDURE IF EXISTS `supla_update_channel_properties` */;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8mb4 */ ;
/*!50003 SET character_set_results = utf8mb4 */ ;
/*!50003 SET collation_connection  = utf8mb4_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = 'STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION' */ ;
DELIMITER ;;
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_channel_properties`(
    IN `_id` INT,
    IN `_user_id` INT,
    IN `_properties` VARCHAR(2048)
)
    NO SQL
BEGIN
    UPDATE supla_dev_channel SET properties = _properties WHERE id = _id AND user_id = _user_id;
END ;;
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
CREATE DEFINER=`supla`@`localhost` PROCEDURE `supla_update_iodevice`(IN `_name` VARCHAR(100) CHARSET utf8mb4, IN `_last_ipv4` INT(10) UNSIGNED, 
  IN `_software_version` VARCHAR(20) CHARSET utf8, IN `_protocol_version` INT(11), IN `_original_location_id` INT(11), 
  IN `_auth_key` VARCHAR(64) CHARSET utf8, IN `_id` INT(11), IN `_flags` INT(11))
    NO SQL
BEGIN
UPDATE `supla_iodevice`
SET
`name` = _name,
`last_connected` = UTC_TIMESTAMP(),
`last_ipv4` = _last_ipv4,
`software_version` = _software_version,
`protocol_version` = _protocol_version,
original_location_id = _original_location_id,
`flags` = IFNULL(`flags`, 0) | IFNULL(_flags, 0) WHERE `id` = _id;
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
-- Final view structure for view `supla_v_accessid_active`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_accessid_active`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_accessid_active`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_general_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`supla`@`localhost` SQL SECURITY DEFINER */
/*!50001 VIEW `supla_v_accessid_active` AS select `sa`.`id` AS `id`,`sa`.`user_id` AS `user_id`,`sa`.`password` AS `password`,`sa`.`caption` AS `caption`,`sa`.`enabled` AS `enabled`,`sa`.`active_from` AS `active_from`,`sa`.`active_to` AS `active_to`,`sa`.`active_hours` AS `active_hours`,`supla_is_now_active`(`sa`.`active_from`,`sa`.`active_to`,`sa`.`active_hours`,`su`.`timezone`) AS `is_now_active` from (`supla_accessid` `sa` join `supla_user` `su` on(`su`.`id` = `sa`.`user_id`)) */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

--
-- Final view structure for view `supla_v_auto_gate_closing`
--

/*!50001 DROP TABLE IF EXISTS `supla_v_auto_gate_closing`*/;
/*!50001 DROP VIEW IF EXISTS `supla_v_auto_gate_closing`*/;
/*!50001 SET @saved_cs_client          = @@character_set_client */;
/*!50001 SET @saved_cs_results         = @@character_set_results */;
/*!50001 SET @saved_col_connection     = @@collation_connection */;
/*!50001 SET character_set_client      = utf8mb4 */;
/*!50001 SET character_set_results     = utf8mb4 */;
/*!50001 SET collation_connection      = utf8mb4_unicode_ci */;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50001 VIEW `supla_v_auto_gate_closing` AS select `c`.`user_id` AS `user_id`,`c`.`enabled` AS `enabled`,`dc`.`iodevice_id` AS `device_id`,`c`.`channel_id` AS `channel_id`,`supla_is_now_active`(`c`.`active_from`,`c`.`active_to`,`c`.`active_hours`,`u`.`timezone`) AS `is_now_active`,`c`.`max_time_open` AS `max_time_open`,`c`.`seconds_open` AS `seconds_open`,`c`.`closing_attempt` AS `closing_attempt`,`c`.`last_seen_open` AS `last_seen_open` from ((`supla_auto_gate_closing` `c` join `supla_user` `u`) join `supla_dev_channel` `dc`) where `c`.`user_id` = `u`.`id` and `c`.`channel_id` = `dc`.`id` */;
/*!50001 SET character_set_client      = @saved_cs_client */;
/*!50001 SET character_set_results     = @saved_cs_results */;
/*!50001 SET collation_connection      = @saved_col_connection */;

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
/*!50001 VIEW `supla_v_client_channel` AS select `c`.`id` AS `id`,`c`.`type` AS `type`,`c`.`func` AS `func`,ifnull(`c`.`param1`,0) AS `param1`,ifnull(`c`.`param2`,0) AS `param2`,`c`.`caption` AS `caption`,ifnull(`c`.`param3`,0) AS `param3`,ifnull(`c`.`param4`,0) AS `param4`,`c`.`text_param1` AS `text_param1`,`c`.`text_param2` AS `text_param2`,`c`.`text_param3` AS `text_param3`,ifnull(`d`.`manufacturer_id`,0) AS `manufacturer_id`,ifnull(`d`.`product_id`,0) AS `product_id`,ifnull(`c`.`user_icon_id`,0) AS `user_icon_id`,`c`.`user_id` AS `user_id`,`c`.`channel_number` AS `channel_number`,`c`.`iodevice_id` AS `iodevice_id`,`cl`.`id` AS `client_id`,case ifnull(`c`.`location_id`,0) when 0 then `d`.`location_id` else `c`.`location_id` end AS `location_id`,ifnull(`c`.`alt_icon`,0) AS `alt_icon`,`d`.`protocol_version` AS `protocol_version`,ifnull(`c`.`flags`,0) | ifnull(`em_subc`.`flags`,0) & 0x020000 | ifnull(`em_subc`.`flags`,0) & 0x040000 | ifnull(`em_subc`.`flags`,0) & 0x080000 AS `flags`,`v`.`value` AS `value`,time_to_sec(timediff(`v`.`valid_to`,utc_timestamp())) AS `validity_time_sec` from (((((((`supla_dev_channel` `c` join `supla_iodevice` `d` on(`d`.`id` = `c`.`iodevice_id`)) join `supla_location` `l` on(`l`.`id` = case ifnull(`c`.`location_id`,0) when 0 then `d`.`location_id` else `c`.`location_id` end)) join `supla_rel_aidloc` `r` on(`r`.`location_id` = `l`.`id`)) join `supla_accessid` `a` on(`a`.`id` = `r`.`access_id`)) join `supla_client` `cl` on(`cl`.`access_id` = `r`.`access_id`)) left join `supla_dev_channel_value` `v` on(`c`.`id` = `v`.`channel_id` and `v`.`valid_to` >= utc_timestamp())) left join `supla_dev_channel` `em_subc` on(`em_subc`.`user_id` = `c`.`user_id` and `em_subc`.`type` = 5000 and (`c`.`func` = 130 or `c`.`func` = 140) and `c`.`param1` = `em_subc`.`id`)) where (`c`.`func` is not null and `c`.`func` <> 0 or `c`.`type` = 8000) and ifnull(`c`.`hidden`,0) = 0 and `d`.`enabled` = 1 and `l`.`enabled` = 1 and `a`.`enabled` = 1 */;
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

-- Dump completed on 2022-10-24 10:59:12
